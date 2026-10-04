#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ASTDugum* ast_olustur(ASTTipi tip, int satir) {
    ASTDugum *d = calloc(1, sizeof(ASTDugum));
    d->tip = tip;
    d->satir = satir;
    return d;
}

ASTListe* ast_liste_olustur(void) {
    ASTListe *l = malloc(sizeof(ASTListe));
    l->kapasite = 4;
    l->sayi = 0;
    l->dugumler = malloc(sizeof(ASTDugum*) * l->kapasite);
    return l;
}

void ast_liste_ekle(ASTListe *liste, ASTDugum *dugum) {
    if (liste->sayi >= liste->kapasite) {
        liste->kapasite *= 2;
        liste->dugumler = realloc(liste->dugumler, 
                                    sizeof(ASTDugum*) * liste->kapasite);
    }
    liste->dugumler[liste->sayi++] = dugum;
}

const char* ast_tip_adi(ASTTipi tip) {
    switch (tip) {
        case AST_SAYI: return "SAYI";
        case AST_METIN: return "METIN";
        case AST_ISIM: return "ISIM";
        case AST_IKILI: return "IKILI";
        case AST_YAZDIR: return "YAZDIR";
        case AST_ATAMA: return "ATAMA";
        case AST_EGER: return "EGER";
        case AST_DONGU: return "DONGU";
        case AST_IKEN: return "IKEN";
        case AST_ISLEV: return "ISLEV";
        case AST_DONDUR: return "DONDUR";
        case AST_BLOK: return "BLOK";
        case AST_SUNUCU: return "SUNUCU";
        case AST_SAYFA: return "SAYFA";
        case AST_HTML_BLOK: return "HTML_BLOK";
        case AST_HTML_ETIKET: return "HTML_ETIKET";
        case AST_VERITABANI: return "VERITABANI";
        case AST_CAGRI: return "CAGRI";
        case AST_ERISIM: return "ERISIM";
        case AST_HER: return "HER";
        default: return "?";
    }
}

void ast_yazdir(ASTDugum *d, int g) {
    if (!d) return;
    for (int i = 0; i < g; i++) printf("  ");
    printf("%s", ast_tip_adi(d->tip));
    
    switch (d->tip) {
        case AST_SAYI:
            printf(" (%g)", d->veri.sayi.deger);
            break;
        case AST_METIN:
            printf(" (\"%s\")", d->veri.metin.metin);
            break;
        case AST_ISIM:
            printf(" (%s)", d->veri.isim.isim);
            break;
        case AST_IKILI:
            printf(" (op=%d)", d->veri.ikili.op);
            break;
        default: break;
    }
    printf("\n");
    
    /* Çocuklar */
    switch (d->tip) {
        case AST_IKILI:
            ast_yazdir(d->veri.ikili.sol, g+1);
            ast_yazdir(d->veri.ikili.sag, g+1);
            break;
        case AST_YAZDIR:
            ast_yazdir(d->veri.yazdir.ifade, g+1);
            break;
        case AST_ATAMA:
            ast_yazdir(d->veri.atama.deger, g+1);
            break;
        case AST_BLOK:
            for (int i = 0; i < d->veri.blok.deyimler->sayi; i++)
                ast_yazdir(d->veri.blok.deyimler->dugumler[i], g+1);