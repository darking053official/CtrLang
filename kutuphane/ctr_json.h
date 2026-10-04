#ifndef CTR_JSON_H
#define CTR_JSON_H

#include "cJSON.h"

/* Yardımcı fonksiyonlar (çakışmayı önlemek için ctr_cjson_ öneki) */
cJSON* ctr_cjson_olustur(void);
void   ctr_cjson_ekle(cJSON *kok, const char *anahtar, const char *deger);
void   ctr_cjson_ekle_sayi(cJSON *kok, const char *anahtar, double deger);
char*  ctr_cjson_yaz(cJSON *kok);
void   ctr_cjson_sil(cJSON *kok);

#endif
