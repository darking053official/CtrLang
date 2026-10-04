#ifndef CTR_LEXER_H
#define CTR_LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_HATA,
    
    /* Değişmezler */
    TOKEN_SAYI,
    TOKEN_METIN,
    TOKEN_ISIM,
    
    /* Anahtar kelimeler */
    TOKEN_YAZDIR,
    TOKEN_SAYI_TIP,
    TOKEN_METIN_TIP,
    TOKEN_MANTIK_TIP,
    TOKEN_EGER,
    TOKEN_DEGILSE,
    TOKEN_IKEN,
    TOKEN_DONGU,
    TOKEN_ISLEV,
    TOKEN_DONDUR,
    TOKEN_DUR,
    TOKEN_DEVAM,
    TOKEN_VE,
    TOKEN_VEYA,
    TOKEN_DEGIL,
    TOKEN_DOGRU,
    TOKEN_YANLIS,
    TOKEN_SUNUCU,
    TOKEN_BASLAT,
    TOKEN_SAYFA,
    TOKEN_HTML,
    TOKEN_JSON,
    TOKEN_VERITABANI,
    TOKEN_BAGLAN,
    TOKEN_SORGU,
    TOKEN_CALISTIR,
    TOKEN_YONLENDIR,
    TOKEN_DURUM,
    TOKEN_ISTEK,
    TOKEN_FORM,
    TOKEN_TIP,
    TOKEN_ICINDE,
    TOKEN_HER,
    
    /* HTML etiketleri */
    TOKEN_BASLIK,
    TOKEN_PARAGRAF,
    TOKEN_DUGME,
    TOKEN_GIRDI,
    TOKEN_KUTU,
    TOKEN_LISTE,
    TOKEN_BAGLANTI,
    TOKEN_RESIM,
    
    /* Operatörler */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_YILDIZ,
    TOKEN_BOLU,
    TOKEN_ESIT,
    TOKEN_ESITTIR,
    TOKEN_ESIT_DEGIL,
    TOKEN_KUCUK,
    TOKEN_BUYUK,
    TOKEN_KUCUK_ESIT,
    TOKEN_BUYUK_ESIT,
    
    /* Ayraçlar */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_VIRGUL,
    TOKEN_NOKTA,
    TOKEN_IKI_NOKTA,
    TOKEN_SLASH,
    TOKEN_SUSLE_PARANTEZ
} TokenTipi;

typedef struct {
    TokenTipi tip;
    const char *baslangic;
    size_t uzunluk;
    int satir;
    int sutun;
    double sayi_deger;
} Token;

typedef struct {
    const char *kaynak;
    const char *baslangic;
    const char *aktif;
    int satir;
    int sutun;
} Lexer;

void lexer_baslat(Lexer *lexer, const char *kaynak);
Token lexer_sonraki(Lexer *lexer);
const char* token_tip_adi(TokenTipi tip);

#endif