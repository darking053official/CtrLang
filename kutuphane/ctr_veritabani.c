#include "ctr_veritabani.h"
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3 *db = NULL;
static ctr_liste *son_sorgu = NULL;

void ctr_veritabani_baglan(const char *dosya) {
    if (db) sqlite3_close(db);
    int rc = sqlite3_open(dosya, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB] Açılamadı: %s\n", sqlite3_errmsg(db));
        db = NULL;
        return;
    }
    printf("[DB] Bağlandı: %s\n", dosya);
}

void ctr_veritabani_kapat(void) {
    if (son_sorgu) { ctr_liste_sil(son_sorgu); son_sorgu = NULL; }
    if (db) { sqlite3_close(db); db = NULL; }
}

void ctr_veritabani_calistir(const char *sql) {
    if (!db) return;
    char *hata = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &hata);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB] %s\n", hata);
        sqlite3_free(hata);
    }
}

static int sorgu_callback(void *veri, int sutun_sayi, char **degerler,
                            char **sutun_isimleri) {
    ctr_liste *l = (ctr_liste*)veri;
    if (!l) return 0;

    /* Satırı JSON string olarak ekle: {a:1,b:2} */
    char satir[8192] = {0};
    strcat(satir, "{");
    for (int i = 0; i < sutun_sayi; i++) {
        if (i > 0) strcat(satir, ",");
        char temp[1024];
        snprintf(temp, sizeof(temp), "\"%s\":\"%s\"",
                 sutun_isimleri[i],
                 degerler[i] ? degerler[i] : "");
        strcat(satir, temp);
    }
    strcat(satir, "}");
    ctr_liste_ekle(l, satir);
    return 0;
}

ctr_liste* ctr_veritabani_sorgu(const char *sql) {
    if (!db) return ctr_liste_olustur();

    if (son_sorgu) { ctr_liste_sil(son_sorgu); son_sorgu = NULL; }
    son_sorgu = ctr_liste_olustur();

    char *hata = NULL;
    int rc = sqlite3_exec(db, sql, sorgu_callback, son_sorgu, &hata);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB] %s\n", hata);
        sqlite3_free(hata);
    }
    return son_sorgu;
}

int ctr_veritabani_satir_sayi(void) {
    return son_sorgu ? son_sorgu->sayi : 0;
}

const char* ctr_veritabani_alan(int satir, const char *alan) {
    (void)satir; (void)alan;
    return "";
}

const char* ctr_veritabani_alan_indeks(int satir, int sutun) {
    (void)satir; (void)sutun;
    return "";
}
