#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static int harf_mi(unsigned char c) {
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c == '_') return 1;
    if (c >= 0x80) return 1;
    return 0;
}

static int rakam_mi(unsigned char c) {
    return c >= '0' && c <= '9';
}

static int bosluk_mi(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

static TokenTipi anahtar_kelime(const char *m, size_t u) {
    #define K(ad, tip) if (u == sizeof(ad)-1 && memcmp(m, ad, u) == 0) return tip;

    K("yazdır", TOKEN_YAZDIR)
    K("sayı", TOKEN_SAYI_TIP)
    K("metin", TOKEN_METIN_TIP)
    K("mantık", TOKEN_MANTIK_TIP)
    K("eğer", TOKEN_EGER)
    K("değilse", TOKEN_DEGILSE)
    K("iken", TOKEN_IKEN)
    K("döngü", TOKEN_DONGU)
    K("işlev", TOKEN_ISLEV)
    K("döndür", TOKEN_DONDUR)
    K("dur", TOKEN_DUR)
    K("devam", TOKEN_DEVAM)
    K("ve", TOKEN_VE)
    K("veya", TOKEN_VEYA)
    K("değil", TOKEN_DEGIL)
    K("doğru", TOKEN_DOGRU)
    K("yanlış", TOKEN_YANLIS)
    K("sunucu", TOKEN_SUNUCU)
    K("başlat", TOKEN_BASLAT)
    K("sayfa", TOKEN_SAYFA)
    K("html", TOKEN_HTML)
    K("json", TOKEN_JSON)
    K("veritabanı", TOKEN_VERITABANI)
    K("bağlan", TOKEN_BAGLAN)
    K("sorgu", TOKEN_SORGU)
    K("çalıştır", TOKEN_CALISTIR)
    K("yönlendir", TOKEN_YONLENDIR)
    K("durum", TOKEN_DURUM)
    K("istek", TOKEN_ISTEK)
    K("form", TOKEN_FORM)
    K("tip", TOKEN_TIP)
    K("içinde", TOKEN_ICINDE)
    K("her", TOKEN_HER)
    K("başlık", TOKEN_BASLIK)
    K("paragraf", TOKEN_PARAGRAF)
    K("düğme", TOKEN_DUGME)
    K("girdi", TOKEN_GIRDI)
    K("kutu", TOKEN_KUTU)
    K("liste", TOKEN_LISTE)
    K("bağlantı", TOKEN_BAGLANTI)
    K("resim", TOKEN_RESIM)

    return TOKEN_ISIM;
    #undef K
}

static Token token_yap(TokenTipi tip, Lexer *lx, const char *bas, size_t u) {
    Token t;
    t.tip = tip;
    t.baslangic = bas;
    t.uzunluk = u;
    t.satir = lx->satir;
    t.sutun = lx->sutun - (int)u;
    t.sayi_deger = 0;
    return t;
}

void lexer_baslat(Lexer *lx, const char *kaynak) {
    lx->kaynak = kaynak;
    lx->baslangic = kaynak;
    lx->aktif = kaynak;
    lx->satir = 1;
    lx->sutun = 1;
}

static char ilerle(Lexer *lx) {
    char c = *lx->aktif++;
    if (c == '\n') {
        lx->satir++;
        lx->sutun = 1;
    } else {
        lx->sutun++;
    }
    return c;
}

static char aktif(Lexer *lx) { return *lx->aktif; }
static char sonraki(Lexer *lx) { return lx->aktif[0] ? lx->aktif[1] : '\0'; }

static void bosluk_atla(Lexer *lx) {
    for (;;) {
        char c = aktif(lx);
        if (bosluk_mi((unsigned char)c) || c == '\n') {
            ilerle(lx);
        } else if (c == '#') {
            while (aktif(lx) != '\n' && aktif(lx) != '\0') ilerle(lx);
        } else if (c == '/' && sonraki(lx) == '/') {
            while (aktif(lx) != '\n' && aktif(lx) != '\0') ilerle(lx);
        } else {
            break;
        }
    }
}

static Token sayi_oku(Lexer *lx) {
    const char *bas = lx->aktif;
    while (rakam_mi((unsigned char)aktif(lx))) ilerle(lx);

    if (aktif(lx) == '.' && rakam_mi((unsigned char)sonraki(lx))) {
        ilerle(lx);
        while (rakam_mi((unsigned char)aktif(lx))) ilerle(lx);
    }

    size_t u = (size_t)(lx->aktif - bas);
    Token t = token_yap(TOKEN_SAYI, lx, bas, u);

    char buf[64];
    size_t kopya = u < 63 ? u : 63;
    memcpy(buf, bas, kopya);
    buf[kopya] = '\0';
    t.sayi_deger = atof(buf);
    return t;
}

static Token metin_oku(Lexer *lx) {
    ilerle(lx);
    const char *bas = lx->aktif;

    while (aktif(lx) != '"' && aktif(lx) != '\0') {
        if (aktif(lx) == '\\' && sonraki(lx)) {
            ilerle(lx);
        }
        ilerle(lx);
    }

    size_t u = (size_t)(lx->aktif - bas);
    if (aktif(lx) == '"') ilerle(lx);

    return token_yap(TOKEN_METIN, lx, bas, u);
}

static Token isim_oku(Lexer *lx) {
    const char *bas = lx->aktif;

    while (harf_mi((unsigned char)aktif(lx)) ||
           rakam_mi((unsigned char)aktif(lx))) {
        if ((unsigned char)aktif(lx) >= 0x80) {
            ilerle(lx);
            if ((unsigned char)aktif(lx) >= 0x80) ilerle(lx);
        } else {
            ilerle(lx);
        }
    }

    size_t u = (size_t)(lx->aktif - bas);
    TokenTipi tip = anahtar_kelime(bas, u);
    return token_yap(tip, lx, bas, u);
}

Token lexer_sonraki(Lexer *lx) {
    bosluk_atla(lx);

    if (aktif(lx) == '\0') {
        return token_yap(TOKEN_EOF, lx, lx->aktif, 0);
    }

    char c = aktif(lx);

    if (rakam_mi((unsigned char)c)) return sayi_oku(lx);
    if (c == '"') return metin_oku(lx);
    if (harf_mi((unsigned char)c)) return isim_oku(lx);

    ilerle(lx);

    switch (c) {
        case '+': return token_yap(TOKEN_PLUS, lx, lx->aktif-1, 1);
        case '-': return token_yap(TOKEN_MINUS, lx, lx->aktif-1, 1);
        case '*': return token_yap(TOKEN_YILDIZ, lx, lx->aktif-1, 1);
        case '/': return token_yap(TOKEN_BOLU, lx, lx->aktif-1, 1);
        case '(': return token_yap(TOKEN_LPAREN, lx, lx->aktif-1, 1);
        case ')': return token_yap(TOKEN_RPAREN, lx, lx->aktif-1, 1);
        case '{': return token_yap(TOKEN_LBRACE, lx, lx->aktif-1, 1);
        case '}': return token_yap(TOKEN_RBRACE, lx, lx->aktif-1, 1);
        case '[': return token_yap(TOKEN_LBRACKET, lx, lx->aktif-1, 1);
        case ']': return token_yap(TOKEN_RBRACKET, lx, lx->aktif-1, 1);
        case ',': return token_yap(TOKEN_VIRGUL, lx, lx->aktif-1, 1);
        case '.': return token_yap(TOKEN_NOKTA, lx, lx->aktif-1, 1);
        case ':': return token_yap(TOKEN_IKI_NOKTA, lx, lx->aktif-1, 1);

        case '=':
            if (aktif(lx) == '=') { ilerle(lx); return token_yap(TOKEN_ESITTIR, lx, lx->aktif-2, 2); }
            return token_yap(TOKEN_ESIT, lx, lx->aktif-1, 1);

        case '!':
            if (aktif(lx) == '=') { ilerle(lx); return token_yap(TOKEN_ESIT_DEGIL, lx, lx->aktif-2, 2); }
            break;

        case '<':
            if (aktif(lx) == '=') { ilerle(lx); return token_yap(TOKEN_KUCUK_ESIT, lx, lx->aktif-2, 2); }
            return token_yap(TOKEN_KUCUK, lx, lx->aktif-1, 1);

        case '>':
            if (aktif(lx) == '=') { ilerle(lx); return token_yap(TOKEN_BUYUK_ESIT, lx, lx->aktif-2, 2); }
            return token_yap(TOKEN_BUYUK, lx, lx->aktif-1, 1);
    }

    return token_yap(TOKEN_HATA, lx, lx->aktif-1, 1);
}

const char* token_tip_adi(TokenTipi tip) {
    switch (tip) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_SAYI: return "SAYI";
        case TOKEN_METIN: return "METIN";
        case TOKEN_ISIM: return "ISIM";
        case TOKEN_YAZDIR: return "YAZDIR";
        case TOKEN_SAYI_TIP: return "SAYI_TIP";
        case TOKEN_METIN_TIP: return "METIN_TIP";
        case TOKEN_EGER: return "EGER";
        case TOKEN_DEGILSE: return "DEGILSE";
        case TOKEN_DONGU: return "DONGU";
        case TOKEN_IKEN: return "IKEN";
        case TOKEN_ISLEV: return "ISLEV";
        case TOKEN_DONDUR: return "DONDUR";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_YILDIZ: return "YILDIZ";
        case TOKEN_BOLU: return "BOLU";
        case TOKEN_ESIT: return "ESIT";
        case TOKEN_ESITTIR: return "ESITTIR";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SUNUCU: return "SUNUCU";
        case TOKEN_BASLAT: return "BASLAT";
        case TOKEN_SAYFA: return "SAYFA";
        case TOKEN_HTML: return "HTML";
        case TOKEN_JSON: return "JSON";
        case TOKEN_VERITABANI: return "VERITABANI";
        case TOKEN_BAGLAN: return "BAGLAN";
        case TOKEN_SORGU: return "SORGU";
        case TOKEN_CALISTIR: return "CALISTIR";
        case TOKEN_YONLENDIR: return "YONLENDIR";
        case TOKEN_DURUM: return "DURUM";
        case TOKEN_ISTEK: return "ISTEK";
        case TOKEN_FORM: return "FORM";
        case TOKEN_TIP: return "TIP";
        case TOKEN_HER: return "HER";
        case TOKEN_ICINDE: return "ICINDE";
        case TOKEN_BASLIK: return "BASLIK";
        case TOKEN_PARAGRAF: return "PARAGRAF";
        case TOKEN_DUGME: return "DUGME";
        case TOKEN_GIRDI: return "GIRDI";
        case TOKEN_KUTU: return "KUTU";
        case TOKEN_LISTE: return "LISTE";
        case TOKEN_BAGLANTI: return "BAGLANTI";
        case TOKEN_RESIM: return "RESIM";
        case TOKEN_VIRGUL: return "VIRGUL";
        case TOKEN_NOKTA: return "NOKTA";
        case TOKEN_IKI_NOKTA: return "IKI_NOKTA";
        case TOKEN_HATA: return "HATA";
        default: return "?";
    }
}
