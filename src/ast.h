#ifndef CTR_AST_H
#define CTR_AST_H

#include "lexer.h"

typedef enum {
    /* İfadeler */
    AST_SAYI,
    AST_METIN,
    AST_DOGRU,
    AST_YANLIS,
    AST_ISIM,
    AST_IKILI,
    AST_TEKLI,
    AST_CAGRI,
    AST_ERISIM,
    AST_INDEKS,
    
    /* Deyimler */
    AST_YAZDIR,
    AST_ATAMA,
    AST_IFADE_STMT,
    AST_EGER,
    AST_DONGU,
    AST_IKEN,
    AST_ISLEV,
    AST_DONDUR,
    AST_DUR,
    AST_DEVAM,
    AST_BLOK,
    
    /* Web */
    AST_SUNUCU,
    AST_SAYFA,
    AST_HTML_BLOK,
    AST_JSON_BLOK,
    AST_VERITABANI,
    AST_YONLENDIR,
    AST_DURUM,
    AST_HER,
    
    /* HTML */
    AST_HTML_ETIKET
} ASTTipi;

typedef struct ASTDugum ASTDugum;
typedef struct ASTListe ASTListe;

struct ASTListe {
    ASTDugum **dugumler;
    int sayi;
    int kapasite;
};

struct ASTDugum {
    ASTTipi tip;
    int satir;
    
    union {
        /* Literaller */
        struct {
            double deger;
        } sayi;
        
        struct {
            char *metin;
            size_t uzunluk;
        } metin;
        
        struct {
            char *isim;
        } isim;
        
        /* İkili işlem */
        struct {
            TokenTipi op;
            ASTDugum *sol;
            ASTDugum *sag;
        } ikili;
        
        /* Tekli işlem */
        struct {
            TokenTipi op;
            ASTDugum *ifade;
        } tekli;
        
        /* Atama */
        struct {
            char *isim;
            ASTDugum *deger;
            char *tip;  /* "sayı", "metin" - opsiyonel */
        } atama;
        
        /* Yazdır */
        struct {
            ASTDugum *ifade;
        } yazdir;
        
        /* Çağrı */
        struct {
            ASTDugum *hedef;
            ASTListe *argumanlar;
        } cagri;
        
        /* Erişim (a.b) */
        struct {
            ASTDugum *nesne;
            char *alan;
        } erisim;
        
        /* İndeks (a[b]) */
        struct {
            ASTDugum *nesne;
            ASTDugum *indeks;
        } indeks;
        
        /* Eğer */
        struct {
            ASTDugum *kosul;
            ASTDugum *o_zaman;
            ASTDugum *degilse;
        } eger;
        
        /* Döngü */
        struct {
            ASTDugum *baslangic;
            ASTDugum *kosul;
            ASTDugum *artis;
            ASTDugum *govde;
        } dongu;
        
        /* İken */
        struct {
            ASTDugum *kosul;
            ASTDugum *govde;
        } iken;
        
        /* İşlev */
        struct {
            char *isim;
            ASTListe *parametreler;
            ASTDugum *govde;
        } islev;
        
        /* Döndür */
        struct {
            ASTDugum *ifade;
        } dondur;
        
        /* Blok */
        struct {
            ASTListe *deyimler;
        } blok;
        
        /* Sunucu başlat(port) */
        struct {
            ASTDugum *port;
        } sunucu;
        
        /* Sayfa "/yol" { ... } */
        struct {
            char *yol;
            char *yontem;  /* GET, POST */
            ASTDugum *govde;
        } sayfa;
        
        /* HTML bloğu */
        struct {
            ASTListe *etiketler;
        } html_blok;
        
        /* HTML etiketi */
        struct {
            TokenTipi etiket;
            char *metin;
            ASTListe *ozellikler;
            ASTDugum *govde;  /* İç içe etiketler */
        } html_etiket;
        
        /* Veritabanı */
        struct {
            char *isim;
            ASTDugum *arg;
        } veritabani;
        
        /* Yönlendir */
        struct {
            char *yol;
        } yonlendir;
        
        /* Durum */
        struct {
            int kod;
        } durum;
        
        /* Her x içinde liste { } */
        struct {
            char *degisken;
            ASTDugum *liste;
            ASTDugum *govde;
        } her;
    } veri;
};

/* Yardımcı */
ASTDugum* ast_olustur(ASTTipi tip, int satir);
ASTListe* ast_liste_olustur(void);
void ast_liste_ekle(ASTListe *liste, ASTDugum *dugum);
void ast_yazdir(ASTDugum *dugum, int girinti);
const char* ast_tip_adi(ASTTipi tip);

#endif