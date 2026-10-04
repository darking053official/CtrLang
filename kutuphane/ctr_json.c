#include "ctr_json.h"
#include <stdlib.h>

cJSON* ctr_cjson_olustur(void) {
    return cJSON_CreateObject();
}

void ctr_cjson_ekle(cJSON *kok, const char *anahtar, const char *deger) {
    if (kok) cJSON_AddStringToObject(kok, anahtar, deger);
}

void ctr_cjson_ekle_sayi(cJSON *kok, const char *anahtar, double deger) {
    if (kok) cJSON_AddNumberToObject(kok, anahtar, deger);
}

char* ctr_cjson_yaz(cJSON *kok) {
    if (!kok) return NULL;
    return cJSON_PrintUnformatted(kok);
}

void ctr_cjson_sil(cJSON *kok) {
    if (kok) cJSON_Delete(kok);
}
