#include "parser.h"
#include "hata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ilerle(Parser *p) {
    p->onceki = p->aktif;
    p->aktif = lexer_sonraki(&p->lexer);
}

static int kontrol(Parser *p, TokenTipi tip) {
    return p->aktif.tip == tip;
}

static int esles(Parser *p, TokenTipi tip) {
    if (kontrol(p, tip)) {
        ilerle(p);
        return 1;
    }
    return 0;
}

static Token bekle(Parser *p, TokenTipi tip, const char *mesaj) {
    if (kontrol(p, tip)) {
        Token t = p->aktif;
        ilerle(p);
        return t;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "%s (beklenen: %s, gelen: %s)",
             mesaj, token_tip_adi(tip), token_tip_adi(p->aktif.tip));
    ctr_hata(HATA_PARSER, buf, p->aktif.satir, p->aktif.sutun);
    return p->aktif;
}

/* İleri bildirim */
static ASTDugum* ifade(Parser *p);
static ASTDugum* deyim(Parser *p);
static ASTDugum* blok(Parser *p);

/* Token'dan metin kopyala */
static char* token_metin(Parser *p, Token t) {
    return arena_strndup(p->arena, t.baslangic, t.uzunluk);
}

/* --- İfadeler --- */

static ASTDugum* birincil(Parser *p) {
    Token t = p->aktif;
    
    /* Sayı */
    if (esles(p, TOKEN_SAYI)) {
        ASTDugum *d = ast_olustur(AST_SAYI, t.satir);
        d->veri.sayi.deger = t.sayi_deger;
        return d;
    }
    
    /* Metin */
    if (esles(p, TOKEN_METIN)) {
        ASTDugum *d = ast_olustur(AST_METIN, t.satir);
        d->veri.metin.metin = token_metin(p, t);
        d->veri.metin.uzunluk = t.uzunluk;
        return d;
    }
    
    /* Doğru / Yanlış */
    if (esles(p, TOKEN_DOGRU)) {
        return ast_olustur(AST_DOGRU, t.satir);
    }
    if (esles(p, TOKEN_YANLIS)) {
        return ast_olustur(AST_YANLIS, t.satir);
    }
    
    /* İsim */
    if (esles(p, TOKEN_ISIM)) {
        ASTDugum *d = ast_olustur(AST_ISIM, t.satir);
        d->veri.isim.isim = token_metin(p, t);
        
        /* Çağrı: isim(...) */
        if (kontrol(p, TOKEN_LPAREN)) {
            ilerle(p);
            ASTListe *args = ast_liste_olustur();
            if (!kontrol(p, TOKEN_RPAREN)) {
                do {
                    ast_liste_ekle(args, ifade(p));
                } while (esles(p, TOKEN_VIRGUL));
            }
            bekle(p, TOKEN_RPAREN, "')' bekleniyor");
            
            ASTDugum *c = ast_olustur(AST_CAGRI, t.satir);
            c->veri.cagri.hedef = d;
            c->veri.cagri.argumanlar = args;
            return c;
        }
        
        /* Erişim: isim.alan */
        while (esles(p, TOKEN_NOKTA)) {
            Token alan = bekle(p, TOKEN_ISIM, "Alan adı bekleniyor");
            ASTDugum *e = ast_olustur(AST_ERISIM, t.satir);
            e->veri.erisim.nesne = d;
            e->veri.erisim.alan = token_metin(p, alan);
            d = e;
        }
        
        /* İndeks: isim[ifade] */
        while (esles(p, TOKEN_LBRACKET)) {
            ASTDugum *i = ifade(p);
            bekle(p, TOKEN_RBRACKET, "']' bekleniyor");
            ASTDugum *idx = ast_olustur(AST_INDEKS, t.satir);
            idx->veri.indeks.nesne = d;
            idx->veri.indeks.indeks = i;
            d = idx;
        }
        
        return d;
    }
    
    /* Parantez */
    if (esles(p, TOKEN_LPAREN)) {
        ASTDugum *d = ifade(p);
        bekle(p, TOKEN_RPAREN, "')' bekleniyor");
        return d;
    }
    
    /* HTML etiketleri */
    if (t.tip == TOKEN_BASLIK || t.tip == TOKEN_PARAGRAF ||
        t.tip == TOKEN_DUGME || t.tip == TOKEN_GIRDI ||
        t.tip == TOKEN_KUTU || t.tip == TOKEN_LISTE ||
        t.tip == TOKEN_BAGLANTI || t.tip == TOKEN_RESIM) {
        ilerle(p);
        ASTDugum *d = ast_olustur(AST_HTML_ETIKET, t.satir);
        d->veri.html_etiket.etiket = t.tip;
        d->veri.html_etiket.metin = NULL;
        d->veri.html_etiket.ozellikler = ast_liste_olustur();
        d->veri.html_etiket.govde = NULL;
        
        /* Metin argümanı */
        if (kontrol(p, TOKEN_METIN) || kontrol(p, TOKEN_ISIM) ||
            kontrol(p, TOKEN_SAYI)) {
            d->veri.html_etiket.metin = token_metin(p, p->aktif);
            ilerle(p);
        }
        
        /* { ... } iç içe */
        if (esles(p, TOKEN_LBRACE)) {
            ASTListe *ic = ast_liste_olustur();
            while (!kontrol(p, TOKEN_RBRACE) && !kontrol(p, TOKEN_EOF)) {
                ast_liste_ekle(ic, deyim(p));
            }
            bekle(p, TOKEN_RBRACE, "'}' bekleniyor");
            
            ASTDugum *b = ast_olustur(AST_BLOK, t.satir);
            b->veri.blok.deyimler = ic;
            d->veri.html_etiket.govde = b;
        }
        
        return d;
    }
    
    ctr_hata(HATA_PARSER, "İfade bekleniyor", t.satir, t.sutun);
    return NULL;
}

/* Çarpma, bölme */
static ASTDugum* carpma(Parser *p) {
    ASTDugum *sol = birincil(p);
    
    while (kontrol(p, TOKEN_YILDIZ) || kontrol(p, TOKEN_BOLU)) {
        Token op = p->aktif;
        ilerle(p);
        ASTDugum *sag = birincil(p);
        ASTDugum *d = ast_olustur(AST_IKILI, op.satir);
        d->veri.ikili.op = op.tip;
        d->veri.ikili.sol = sol;
        d->veri.ikili.sag = sag;
        sol = d;
    }
    return sol;
}

/* Toplama, çıkarma */
static ASTDugum* toplama(Parser *p) {
    ASTDugum *sol = carpma(p);
    
    while (kontrol(p, TOKEN_PLUS) || kontrol(p, TOKEN_MINUS)) {
        Token op = p->aktif;
        ilerle(p);
        ASTDugum *sag = carpma(p);
        ASTDugum *d = ast_olustur(AST_IKILI, op.satir);
        d->veri.ikili.op = op.tip;
        d->veri.ikili.sol = sol;
        d->veri.ikili.sag = sag;
        sol = d;
    }
    return sol;
}

/* Karşılaştırma */
static ASTDugum* karsilastirma(Parser *p) {
    ASTDugum *sol = toplama(p);
    
    while (kontrol(p, TOKEN_ESITTIR) || kontrol(p, TOKEN_ESIT_DEGIL) ||
           kontrol(p, TOKEN_KUCUK) || kontrol(p, TOKEN_BUYUK) ||
           kontrol(p, TOKEN_KUCUK_ESIT) || kontrol(p, TOKEN_BUYUK_ESIT)) {
        Token op = p->aktif;
        ilerle(p);
        ASTDugum *sag = toplama(p);
        ASTDugum *d = ast_olustur(AST_IKILI, op.satir);
        d->veri.ikili.op = op.tip;
        d->veri.ikili.sol = sol;
        d->veri.ikili.sag = sag;
        sol = d;
    }
    return sol;
}

static ASTDugum* ifade(Parser *p) {
    return karsilastirma(p);
}

/* --- Blok ve deyimler --- */

static ASTDugum* blok(Parser *p) {
    Token t = bekle(p, TOKEN_LBRACE, "'{' bekleniyor");
    ASTListe *liste = ast_liste_olustur();
    
    while (!kontrol(p, TOKEN_RBRACE) && !kontrol(p, TOKEN_EOF)) {
        ast_liste_ekle(liste, deyim(p));
    }
    
    bekle(p, TOKEN_RBRACE, "'}' bekleniyor");
    
    ASTDugum *d = ast_olustur(AST_BLOK, t.satir);
    d->veri.blok.deyimler = liste;
    return d;
}

/* Yazdır */
static ASTDugum* yazdir_parse(Parser *p) {
    Token t = p->onceki;
    ASTDugum *ifade_dugum = ifade(p);
    ASTDugum *d = ast_olustur(AST_YAZDIR, t.satir);
    d->veri.yazdir.ifade = ifade_dugum;
    return d;
}

/* Atama: [tip] isim = ifade */
static ASTDugum* atama_parse(Parser *p, char *tip, Token isim) {
    bekle(p, TOKEN_ESIT, "'=' bekleniyor");
    ASTDugum *deger = ifade(p);
    
    ASTDugum *d = ast_olustur(AST_ATAMA, isim.satir);
    d->veri.atama.isim = token_metin(p, isim);
    d->veri.atama.deger = deger;
    d->veri.atama.tip = tip;
    return d;
}

/* Eğer */
static ASTDugum* eger_parse(Parser *p) {
    Token t = p->onceki;
    ASTDugum *kosul = ifade(p);
    ASTDugum *o_zaman = blok(p);
    
    ASTDugum *degilse = NULL;
    if (esles(p, TOKEN_DEGILSE)) {
        if (kontrol(p, TOKEN_EGER)) {
            ilerle(p);
            degilse = eger_parse(p);
        } else {
            degilse = blok(p);
        }
    }
    
    ASTDugum *d = ast_olustur(AST_EGER, t.satir);
    d->veri.eger.kosul = kosul;
    d->veri.eger.o_zaman = o_zaman;
    d->veri.eger.degilse = degilse;
    return d;
}

/* İken */
static ASTDugum* iken_parse(Parser *p) {
    Token t = p->onceki;
    ASTDugum *kosul = ifade(p);
    ASTDugum *govde = blok(p);
    
    ASTDugum *d = ast_olustur(AST_IKEN, t.satir);
    d->veri.iken.kosul = kosul;
    d->veri.iken.govde = govde;
    return d;
}

/* Döngü: döngü i = 0, i < 10, i = i + 1 { } */
static ASTDugum* dongu_parse(Parser *p) {
    Token t = p->onceki;
    
    /* Başlangıç: tip isim = ifade  veya  isim = ifade */
    ASTDugum *baslangic = NULL;
    if (kontrol(p, TOKEN_SAYI_TIP)) {
        ilerle(p);
        Token isim = bekle(p, TOKEN_ISIM, "Değişken adı bekleniyor");
        baslangic = atama_parse(p, "sayı", isim);
    } else {
        Token isim = bekle(p, TOKEN_ISIM, "Değişken adı bekleniyor");
        baslangic = atama_parse(p, NULL, isim);
    }
    
    bekle(p, TOKEN_VIRGUL, "',' bekleniyor");
    ASTDugum *kosul = ifade(p);
    bekle(p, TOKEN_VIRGUL, "',' bekleniyor");
    
    /* Artış: isim = ifade */
    Token isim = bekle(p, TOKEN_ISIM, "Değişken adı bekleniyor");
    ASTDugum *artis = atama_parse(p, NULL, isim);
    
    ASTDugum *govde = blok(p);
    
    ASTDugum *d = ast_olustur(AST_DONGU, t.satir);
    d->veri.dongu.baslangic = baslangic;
    d->veri.dongu.kosul = kosul;
    d->veri.dongu.artis = artis;
    d->veri.dongu.govde = govde;
    return d;
}

/* İşlev */
static ASTDugum* islev_parse(Parser *p) {
    Token t = p->onceki;
    Token isim = bekle(p, TOKEN_ISIM, "İşlev adı bekleniyor");
    bekle(p, TOKEN_LPAREN, "'(' bekleniyor");
    
    ASTListe *params = ast_liste_olustur();
    if (!kontrol(p, TOKEN_RPAREN)) {
        do {
            Token pt = bekle(p, TOKEN_ISIM, "Parametre adı bekleniyor");
            ASTDugum *pd = ast_olustur(AST_ISIM, pt.satir);
            pd->veri.isim.isim = token_metin(p, pt);
            ast_liste_ekle(params, pd);
        } while (esles(p, TOKEN_VIRGUL));
    }
    bekle(p, TOKEN_RPAREN, "')' bekleniyor");
    
    ASTDugum *govde = blok(p);
    
    ASTDugum *d = ast_olustur(AST_ISLEV, t.satir);
    d->veri.islev.isim = token_metin(p, isim);
    d->veri.islev.parametreler = params;
    d->veri.islev.govde = govde;
    return d;
}

/* Döndür */
static ASTDugum* dondur_parse(Parser *p) {
    Token t = p->onceki;
    ASTDugum *ifd = ifade(p);
    ASTDugum *d = ast_olustur(AST_DONDUR, t.satir);
    d->veri.dondur.ifade = ifd;
    return d;
}

/* Sunucu başlat(port) */
static ASTDugum* sunucu_parse(Parser *p) {
    Token t = p->onceki;
    bekle(p, TOKEN_BASLAT, "'başlat' bekleniyor");
    bekle(p, TOKEN_LPAREN, "'(' bekleniyor");
    ASTDugum *port = ifade(p);
    bekle(p, TOKEN_RPAREN, "')' bekleniyor");
    
    ASTDugum *d = ast_olustur(AST_SUNUCU, t.satir);
    d->veri.sunucu.port = port;
    return d;
}

/* Sayfa "/yol" [yöntem: "POST"] { } */
static ASTDugum* sayfa_parse(Parser *p) {
    Token t = p->onceki;
    Token yol = bekle(p, TOKEN_METIN, "Yol (metin) bekleniyor");
    
    char *yontem = NULL;
    if (esles(p, TOKEN_LBRACKET)) {
        /* [yöntem: "POST"] */
        while (!kontrol(p, TOKEN_RBRACKET) && !kontrol(p, TOKEN_EOF)) {
            ilerle(p);
        }
        bekle(p, TOKEN_RBRACKET, "']' bekleniyor");
    }
    
    /* Blok */
    bekle(p, TOKEN_LBRACE, "'{' bekleniyor");
    ASTListe *liste = ast_liste_olustur();
    while (!kontrol(p, TOKEN_RBRACE) && !kontrol(p, TOKEN_EOF)) {
        ast_liste_ekle(liste, deyim(p));
    }
    bekle(p, TOKEN_RBRACE, "'}' bekleniyor");
    
    ASTDugum *b = ast_olustur(AST_BLOK, t.satir);
    b->veri.blok.deyimler = liste;
    
    ASTDugum *d = ast_olustur(AST_SAYFA, t.satir);
    d->veri.sayfa.yol = token_metin(p, yol);
    d->veri.sayfa.yontem = yontem;
    d->veri.sayfa.govde = b;
    return d;
}

/* HTML bloğu */
static ASTDugum* html_parse(Parser *p) {
    Token t = p->onceki;
    bekle(p, TOKEN_LBRACE, "'{' bekleniyor");
    
    ASTListe *liste = ast_liste_olustur();
    while (!kontrol(p, TOKEN_RBRACE) && !kontrol(p, TOKEN_EOF)) {
        ast_liste_ekle(liste, deyim(p));
    }
    bekle(p, TOKEN_RBRACE, "'}' bekleniyor");
    
    ASTDugum *d = ast_olustur(AST_HTML_BLOK, t.satir);
    d->veri.html_blok.etiketler = liste;
    return d;
}

/* Veritabanı bağlan("x.db") */
static ASTDugum* veritabani_parse(Parser *p) {
    Token t = p->onceki;
    bekle(p, TOKEN_BAGLAN, "'bağlan' bekleniyor");
    bekle(p, TOKEN_LPAREN, "'(' bekleniyor");
    ASTDugum *arg = ifade(p);
    bekle(p, TOKEN_RPAREN, "')' bekleniyor");
    
    ASTDugum *d = ast_olustur(AST_VERITABANI, t.satir);
    d->veri.veritabani.isim = "bağlan";
    d->veri.veritabani.arg = arg;
    return d;
}

/* Yönlendir "/yol" */
static ASTDugum* yonlendir_parse(Parser *p) {
    Token t = p->onceki;
    Token yol = bekle(p, TOKEN_METIN, "Yol (metin) bekleniyor");
    ASTDugum *d = ast_olustur(AST_YONLENDIR, t.satir);
    d->veri.yonlendir.yol = token_metin(p, yol);
    return d;
}

/* Durum 404 */
static ASTDugum* durum_parse(Parser *p) {
    Token t = p->onceki;
    Token kod = bekle(p, TOKEN_SAYI, "Durum kodu bekleniyor");
    ASTDugum *d = ast_olustur(AST_DURUM, t.satir);
    d->veri.durum.kod = (int)kod.sayi_deger;
    return d;
}

/* Her x içinde liste { } */
static ASTDugum* her_parse(Parser *p) {
    Token t = p->onceki;
    Token deg = bekle(p, TOKEN_ISIM, "Değişken adı bekleniyor");
    bekle(p, TOKEN_ICINDE, "'içinde' bekleniyor");
    ASTDugum *liste = ifade(p);
    ASTDugum *govde = blok(p);
    
    ASTDugum *d = ast_olustur(AST_HER, t.satir);
    d->veri.her.degisken = token_metin(p, deg);
    d->veri.her.liste = liste;
    d->veri.her.govde = govde;
    return d;
}

/* --- Ana deyim --- */

static ASTDugum* deyim(Parser *p) {
    Token t = p->aktif;
    
    /* Yazdır */
    if (esles(p, TOKEN_YAZDIR)) return yazdir_parse(p);
    
    /* Eğer */
    if (esles(p, TOKEN_EGER)) return eger_parse(p);
    
    /* İken */
    if (esles(p, TOKEN_IKEN)) return iken_parse(p);
    
    /* Döngü */
    if (esles(p, TOKEN_DONGU)) return dongu_parse(p);
    
    /* İşlev */
    if (esles(p, TOKEN_ISLEV)) return islev_parse(p);
    
    /* Döndür */
    if (esles(p, TOKEN_DONDUR)) return dondur_parse(p);
    
    /* Dur / Devam */
    if (esles(p, TOKEN_DUR)) {
        return ast_olustur(AST_DUR, t.satir);
    }
    if (esles(p, TOKEN_DEVAM)) {
        return ast_olustur(AST_DEVAM, t.satir);
    }
    
    /* Sunucu */
    if (esles(p, TOKEN_SUNUCU)) return sunucu_parse(p);
    
    /* Sayfa */
    if (esles(p, TOKEN_SAYFA)) return sayfa_parse(p);
    
    /* HTML */
    if (esles(p, TOKEN_HTML)) return html_parse(p);
    
    /* Veritabanı */
    if (esles(p, TOKEN_VERITABANI)) return veritabani_parse(p);
    
    /* Yönlendir */
    if (esles(p, TOKEN_YONLENDIR)) return yonlendir_parse(p);
    
    /* Durum */
    if (esles(p, TOKEN_DURUM)) return durum_parse(p);
    
    /* Her */
    if (esles(p, TOKEN_HER)) return her_parse(p);
    
    /* Tip + isim = ifade (atama) */
    if (t.tip == TOKEN_SAYI_TIP || t.tip == TOKEN_METIN_TIP ||
        t.tip == TOKEN_MANTIK_TIP) {
        const char *tip_adi = NULL;
        if (t.tip == TOKEN_SAYI_TIP) tip_adi = "sayı";
        else if (t.tip == TOKEN_METIN_TIP) tip_adi = "metin";
        else tip_adi = "mantık";
        
        ilerle(p);
        Token isim = bekle(p, TOKEN_ISIM, "Değişken adı bekleniyor");
        return atama_parse(p, (char*)tip_adi, isim);
    }
    
    /* isim = ifade (atama) */
    if (t.tip == TOKEN_ISIM) {
        Token isim = p->aktif;
        ilerle(p);
        
        /* Erişim veya indeks olabilir: a.b = ... veya a[i] = ... */
        if (kontrol(p, TOKEN_ESIT)) {
            return atama_parse(p, NULL, isim);
        }
        
        /* Diğer ifade */
        p->aktif = isim;
        p->onceki = isim;
        /* Geri al: ifade olarak oku */
        ASTDugum *ifd = ifade(p);
        ASTDugum *d = ast_olustur(AST_IFADE_STMT, t.satir);
        d->veri.yazdir.ifade = ifd;
        return d;
    }
    
    /* Sadece ifade (fonksiyon çağrısı vb.) */
    ASTDugum *ifd = ifade(p);
    ASTDugum *d = ast_olustur(AST_IFADE_STMT, t.satir);
    d->veri.yazdir.ifade = ifd;
    return d;
}

void parser_baslat(Parser *p, const char *kaynak, Arena *arena) {
    lexer_baslat(&p->lexer, kaynak);
    p->arena = arena;
    p->hata_var = 0;
    p->aktif.tip = TOKEN_EOF;
    p->onceki.tip = TOKEN_EOF;
    ilerle(p);  /* İlk token */
}

ASTDugum* parser_calistir(Parser *p) {
    ASTListe *liste = ast_liste_olustur();
    
    while (!kontrol(p, TOKEN_EOF)) {
        ast_liste_ekle(liste, deyim(p));
    }
    
    ASTDugum *kok = ast_olustur(AST_BLOK, 1);
    kok->veri.blok.deyimler = liste;
    return kok;
}