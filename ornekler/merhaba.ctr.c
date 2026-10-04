/* CtrLang tarafından üretildi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctr_calisma.h"

/* Global değişkenler ve fonksiyonlar */

static void sayfa_1(ctr_istek *istek);

int main(int argc, char **argv) {
    ctr_baslat();

    ctr_sunucu_baslat(3000);
    ctr_sayfa_ekle("/", "GET", sayfa_1);

    ctr_sunucu_dongu();
    ctr_bitir();
    return 0;
}

static void sayfa_1(ctr_istek *istek) {
    ctr_yazdir("Merhaba CtrLang!");
}


