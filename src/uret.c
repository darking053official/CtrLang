#include "uret.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>

/* Değişken tipleri takibi için basit bir sistem */
#define MAX_DEGISKEN 1024

typedef struct {
    char isim[128];
    char tip[16];  /* "double", "char*", "int" */
} Degisken;

static Degisken degiskenler[MAX_DEGISKEN];
static int degisken_sayi = 0;

static const char* degisken_tip_bul(const char *isim) {
    for (int i = 0; i < degisken_sayi; i++) {
        if (strcmp(degiskenler[i].isim, isim) == 0) {
            return degiskenler[i].tip;
        }
    }
    return NULL;
}

static void degisken_ekle(const char *isim, const char *tip) {
    if (degisken_sayi >= MAX_DEGISKEN) return;
    strncpy(degiskenler[degisken_sayi].isim, isim, 127);
    strncpy(degiskenler[degisken_sayi].tip, tip, 15);
    degisken_sayi++;
}

static void girinti_yaz(FILE *f, int g) {
    for (int i = 0; i < g; i++) fprintf(f, "    ");
}

void uret_baslat(FILE *f) {
    fprintf(f, "/* CtrLang tarafından üretildi */\n");
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <string.h>\n");
    fprintf(f, "#include \"ctr_calisma.h\"\n\n");
}

void uret_bitir(FILE *f) {
    fprintf(f, "\n");
}

/* İfade üret */
static void uret_ifade(FILE *f, ASTDugum *d) {
    if (!d) { fprintf(f, "0"); return; }
    
    switch (d->tip) {
        case AST_SAYI:
            if (d->veri.sayi.deger == (int)d->veri.sayi.deger)
                fprintf(f, "%d", (int)d->veri.sayi.deger);
            else
                fprintf(f, "%g", d->veri.sayi.deger);
            break;
        
        case AST_METIN:
            fprintf(f, "\"");
            for (size_t i = 0; i < d->veri.metin.uzunluk; i++) {
                char c = d->veri.metin.metin[i];
                if (c == '"' || c == '\\') fprintf(f, "\\%c", c);
                else if (c == '\n') fprintf(f, "\\n");
                else fprintf(f, "%c", c);
            }
            fprintf(f, "\"");
            break;
        
        case AST_DOGRU:
            fprintf(f, "1");
            break;
        
        case AST_YANLIS:
            fprintf(f, "0");
            break;
        
        case AST_ISIM: {
            const char *tip = degisken_tip_bul(d->veri.isim.isim);
            if (tip && strcmp(tip, "double") == 0)
                fprintf(f, "ctr_sayi_metin(%s)", d->veri.isim.isim);
            else
                fprintf(f, "%s", d->veri.isim.isim);
            break;
        }
        
        case AST_IKILI:
            fprintf(f, "(");
            uret_ifade(f, d->veri.ikili.sol);
            switch (d->veri.ikili.op) {
                case TOKEN_PLUS: fprintf(f, " + "); break;
                case TOKEN_MINUS: fprintf(f, " - "); break;
                case TOKEN_YILDIZ: fprintf(f, " * "); break;
                case TOKEN_BOLU: fprintf(f, " / "); break;
                case TOKEN_ESITTIR: fprintf(f, " == "); break;
                case TOKEN_ESIT_DEGIL: fprintf(f, " != "); break;
                case TOKEN_KUCUK: fprintf(f, " < "); break;
                case TOKEN_BUYUK: fprintf(f, " > "); break;
                case TOKEN_KUCUK_ESIT: fprintf(f, " <= "); break;
                case TOKEN_BUYUK_ESIT: fprintf(f, " >= "); break;
                default: fprintf(f, " + "); break;
            }
            uret_ifade(f, d->veri.ikili.sag);
            fprintf(f, ")");
            break;
        
        case AST_CAGRI: {
            /* Basit fonksiyon çağrısı */
            if (d->veri.cagri.hedef->tip == AST_ISIM) {
                fprintf(f, "%s(", d->veri.cagri.hedef->veri.isim.isim);
                for (int i = 0; i < d->veri.cagri.argumanlar->sayi; i++) {
                    if (i > 0) fprintf(f, ", ");
                    uret_ifade(f, d->veri.cagri.argumanlar->dugumler[i]);
                }
                fprintf(f, ")");
            }
            break;
        }
        
        case AST_ERISIM: {
            /* isim.alan → ctr_alan(isim, "alan") */
            uret_ifade(f, d->veri.erisim.nesne);
            fprintf(f, ".%s", d->veri.erisim.alan);
            break;
        }
        
        case AST_HTML_ETIKET:
            /* HTML etiketi ifade olarak kullanılırsa */
            fprintf(f, "\"\"");
            break;
        
        default:
            fprintf(f, "0");
            break;
    }
}

/* Deyim üret */
void uret_dugum(FILE *f, ASTDugum *d, int g) {
    if (!d) return;
    
    switch (d->tip) {
        case AST_BLOK:
            for (int i = 0; i < d->veri.blok.deyimler->sayi; i++) {
                uret_dugum(f, d->veri.blok.deyimler->dugumler[i], g);
            }
            break;
        
        case AST_YAZDIR:
            girinti_yaz(f, g);
            fprintf(f, "ctr_yazdir(");
            uret_ifade(f, d->veri.yazdir.ifade);
            fprintf(f, ");\n");
            break;
        
        case AST_ATAMA: {
            girinti_yaz(f, g);
            
            const char *c_tip = "double";
            if (d->veri.atama.tip) {
                if (strcmp(d->veri.atama.tip, "metin") == 0) c_tip = "char*";
                else if (strcmp(d->veri.atama.tip, "mantık") == 0) c_tip = "int";
            } else {
                /* Değerden tip tahmin et */
                if (d->veri.atama.deger->tip == AST_METIN) c_tip = "char*";
            }
            
            /* İlk kez mi? */
            if (!degisken_tip_bul(d->veri.atama.isim)) {
                fprintf(f, "%s %s = ", c_tip, d->veri.atama.isim);
                degisken_ekle(d->veri.atama.isim, c_tip);
            } else {
                fprintf(f, "%s = ", d->veri.atama.isim);
            }
            
            uret_ifade(f, d->veri.atama.deger);
            fprintf(f, ";\n");
            break;
        }
        
        case AST_IFADE_STMT:
            girinti_yaz(f, g);
            uret_ifade(f, d->veri.yazdir.ifade);
            fprintf(f, ";\n");
            break;
        
        case AST_EGER:
            girinti_yaz(f, g);
            fprintf(f, "if (");
            uret_ifade(f, d->veri.eger.kosul);
            fprintf(f, ") {\n");
            uret_dugum(f, d->veri.eger.o_zaman, g+1);
            girinti_yaz(f, g);
            fprintf(f, "}");
            if (d->veri.eger.degilse) {
                fprintf(f, " else {\n");
                uret_dugum(f, d->veri.eger.degilse, g+1);
                girinti_yaz(f, g);
                fprintf(f, "}");
            }
            fprintf(f, "\n");
            break;
        
        case AST_IKEN:
            girinti_yaz(f, g);
            fprintf(f, "while (");
            uret_ifade(f, d->veri.iken.kosul);
            fprintf(f, ") {\n");
            uret_dugum(f, d->veri.iken.govde, g+1);
            girinti_yaz(f, g);
            fprintf(f, "}\n");
            break;
        
        case AST_DONGU:
            girinti_yaz(f, g);
            fprintf(f, "for (");
            /* Başlangıç */
            if (d->veri.dongu.baslangic) {
                /* int i = 0 */
                ASTDugum *a = d->veri.dongu.baslangic;
                fprintf(f, "double %s = ", a->veri.atama.isim);
                uret_ifade(f, a->veri.atama.deger);
                degisken_ekle(a->veri.atama.isim, "double");
            }
            fprintf(f, "; ");
            uret_ifade(f, d->veri.dongu.kosul);
            fprintf(f, "; ");
            /* Artış */
            if (d->veri.dongu.artis) {
                ASTDugum *a = d->veri.dongu.artis;
                fprintf(f, "%s = ", a->veri.atama.isim);
                uret_ifade(f, a->veri.atama.deger);
            }
            fprintf(f, ") {\n");
            uret_dugum(f, d->veri.dongu.govde, g+1);
            girinti_yaz(f, g);
            fprintf(f, "}\n");
            break;
        
        case AST_ISLEV: {
            girinti_yaz(f, g);
            fprintf(f, "void %s(", d->veri.islev.isim);
            for (int i = 0; i < d->veri.islev.parametreler->sayi; i++) {
                if (i > 0) fprintf(f, ", ");
                ASTDugum *p = d->veri.islev.parametreler->dugumler[i];
                fprintf(f, "double %s", p->veri.isim.isim);
            }
            fprintf(f, ") {\n");
            uret_dugum(f, d->veri.islev.govde, g+1);
            girinti_yaz(f, g);
            fprintf(f, "}\n");
            break;
        }
        
        case AST_DONDUR:
            girinti_yaz(f, g);
            fprintf(f, "return ");
            uret_ifade(f, d->veri.dondur.ifade);
            fprintf(f, ";\n");
            break;
        
        case AST_SUNUCU:
            girinti_yaz(f, g);
            fprintf(f, "ctr_sunucu_baslat(");
            uret_ifade(f, d->veri.sunucu.port);
            fprintf(f, ");\n");
            break;
        
        case AST_SAYFA: {
            /* sayfa "/yol" { } → kayıt */
            girinti_yaz(f, g);
            fprintf(f, "ctr_sayfa_ekle(");
            
            /* Yol */
            fprintf(f, "\"");
            for (size_t i = 0; d->veri.sayfa.yol[i]; i++) {
                char c = d->veri.sayfa.yol[i];
                if (c == '"' || c == '\\') fprintf(f, "\\%c", c);
                else fprintf(f, "%c", c);
            }
            fprintf(f, "\", ");
            
            /* Yöntem */
            if (d->veri.sayfa.yontem)
                fprintf(f, "\"%s\", ", d->veri.sayfa.yontem);
            else
                fprintf(f, "\"GET\", ");
            
            /* Fonksiyon adı */
            static int sayfa_no = 0;
            int bu_no = sayfa_no++;
            fprintf(f, "sayfa_%d);\n\n", bu_no);
            
            /* Fonksiyonu ayrı yaz */
            fprintf(f, "static void sayfa_%d(ctr_istek *istek) {\n", bu_no);
            uret_dugum(f, d->veri.sayfa.govde, 1);
            fprintf(f, "}\n\n");
            break;
        }
        
        case AST_HTML_BLOK: {
            girinti_yaz(f, g);
            fprintf(f, "ctr_html_baslat(istek);\n");
            for (int i = 0; i < d->veri.html_blok.etiketler->sayi; i++) {
                uret_dugum(f, d->veri.html_blok.etiketler->dugumler[i], g);
            }
            girinti_yaz(f, g);
            fprintf(f, "ctr_html_bitir(istek);\n");
            break;
        }
        
        case AST_HTML_ETIKET: {
            girinti_yaz(f, g);
            const char *etiket_adi = NULL;
            switch (d->veri.html_etiket.etiket) {
                case TOKEN_BASLIK: etiket_adi = "h1"; break;
                case TOKEN_PARAGRAF: etiket_adi = "p"; break;
                case TOKEN_DUGME: etiket_adi = "button"; break;
                case TOKEN_GIRDI: etiket_adi = "input"; break;
                case TOKEN_KUTU: etiket_adi = "div"; break;
                case TOKEN_LISTE: etiket_adi = "ul"; break;
                case TOKEN_BAGLANTI: etiket_adi = "a"; break;
                case TOKEN_RESIM: etiket_adi = "img"; break;
                default: etiket_adi = "div"; break;
            }
            
            fprintf(f, "ctr_html_ac(istek, \"%s\");\n", etiket_adi);
            
            if (d->veri.html_etiket.metin) {
                girinti_yaz(f, g);
                fprintf(f, "ctr_html_metin(istek, \"%s\");\n", 
                        d->veri.html_etiket.metin);
            }
            
            if (d->veri.html_etiket.govde) {
                uret_dugum(f, d->veri.html_etiket.govde, g);
            }
            
            girinti_yaz(f, g);
            fprintf(f, "ctr_html_kapat(istek, \"%s\");\n", etiket_adi);
            break;
        }
        
        case AST_VERITABANI:
            girinti_yaz(f, g);
            fprintf(f, "ctr_veritabani_baglan(");
            uret_ifade(f, d->veri.veritabani.arg);
            fprintf(f, ");\n");
            break;
        
        case AST_YONLENDIR:
            girinti_yaz(f, g);
            fprintf(f, "ctr_yonlendir(istek, \"%s\");\n", d->veri.yonlendir.yol);
            break;
        
        case AST_DURUM:
            girinti_yaz(f, g);
            fprintf(f, "ctr_durum(istek, %d);\n", d->veri.durum.kod);
            break;
        
        case AST_HER: {
            girinti_yaz(f, g);
            fprintf(f, "/* her %s içinde */\n", d->veri.her.degisken);
            /* Basit liste yok şimdilik */
            break;
        }
        
        case AST_DUR:
            girinti_yaz(f, g);
            fprintf(f, "break;\n");
            break;
        
        case AST_DEVAM:
            girinti_yaz(f, g);
            fprintf(f, "continue;\n");
            break;
        
        default:
            break;
    }
}

void uret_program(FILE *f, ASTDugum *kok) {
    /* Önce fonksiyonlar ve global değişkenler */
    fprintf(f, "/* Global değişkenler ve fonksiyonlar */\n\n");
    
    /* Sayfa fonksiyonlarını ileri bildir */
    if (kok && kok->tip == AST_BLOK) {
        for (int i = 0; i < kok->veri.blok.deyimler->sayi; i++) {
            ASTDugum *d = kok->veri.blok.deyimler->dugumler[i];
            if (d->tip == AST_SAYFA) {
                fprintf(f, "static void sayfa_%d(ctr_istek *istek);\n", i);
            }
        }
        fprintf(f, "\n");
    }
    
    /* Ana kod (main içinde) */
    fprintf(f, "int main(int argc, char **argv) {\n");
    fprintf(f, "    ctr_baslat();\n\n");
    
    if (kok && kok->tip == AST_BLOK) {
        for (int i = 0; i < kok->veri.blok.deyimler->sayi; i++) {
            ASTDugum *d = kok->veri.blok.deyimler->dugumler[i];
            if (d->tip == AST_SAYFA) {
                /* Yukarıda fonksiyon olarak yazıldı */
                /* Burada sadece kayıt yap */
                girinti_yaz(f, 1);
                fprintf(f, "ctr_sayfa_ekle(\"%s\", \"%s\", sayfa_%d);\n",
                        d->veri.sayfa.yol,
                        d->veri.sayfa.yontem ? d->veri.sayfa.yontem : "GET",
                        i);
            } else if (d->tip == AST_ISLEV) {
                /* İşlevi main dışında yaz */
                continue;
            } else {
                uret_dugum(f, d, 1);
            }
        }
    }
    
    fprintf(f, "\n    ctr_sunucu_dongu();\n");
    fprintf(f, "    ctr_bitir();\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n\n");
    
    /* Sayfa fonksiyonlarını yaz */
    if (kok && kok->tip == AST_BLOK) {
        for (int i = 0; i < kok->veri.blok.deyimler->sayi; i++) {
            ASTDugum *d = kok->veri.blok.deyimler->dugumler[i];
            if (d->tip == AST_SAYFA) {
                fprintf(f, "static void sayfa_%d(ctr_istek *istek) {\n", i);
                uret_dugum(f, d->veri.sayfa.govde, 1);
                fprintf(f, "}\n\n");
            }
        }
    }
}