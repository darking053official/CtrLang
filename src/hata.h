#ifndef CTR_HATA_H
#define CTR_HATA_H

typedef enum {
    HATA_LEXER = 1,
    HATA_PARSER,
    HATA_URET,
    HATA_BELLEK,
    HATA_DOSYA,
    HATA_CALISMA
} HataTipi;

void ctr_hata(HataTipi tip, const char *mesaj, int satir, int sutun);
void ctr_hata_basit(HataTipi tip, const char *mesaj);
void ctr_uyari(const char *mesaj, int satir, int sutun);

#endif