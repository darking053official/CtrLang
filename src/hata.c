#include "hata.h"
#include <stdio.h>
#include <stdlib.h>

static const char* tip_adi(HataTipi tip) {
    switch (tip) {
        case HATA_LEXER:    return "Lexer";
        case HATA_PARSER:   return "Parser";
        case HATA_URET:     return "Üretici";
        case HATA_BELLEK:   return "Bellek";
        case HATA_DOSYA:    return "Dosya";
        case HATA_CALISMA:  return "Çalışma";
        default:            return "Hata";
    }
}

void ctr_hata(HataTipi tip, const char *mesaj, int satir, int sutun) {
    fprintf(stderr, "\n[HATA:%s] %d:%d\n  %s\n\n",
            tip_adi(tip), satir, sutun, mesaj);
    exit(1);
}

void ctr_hata_basit(HataTipi tip, const char *mesaj) {
    fprintf(stderr, "\n[HATA:%s]\n  %s\n\n", tip_adi(tip), mesaj);
    exit(1);
}

void ctr_uyari(const char *mesaj, int satir, int sutun) {
    fprintf(stderr, "[UYARI] %d:%d: %s\n", satir, sutun, mesaj);
}