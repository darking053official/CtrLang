#include "ctr_veritabani.h"
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SATIR 4096
#define MAX_SUTUN 64

static sqlite3 *db = NULL;

static struct {
    int satir_sayi;
    int sutun_sayi;
    char sutun_isim[MAX_SUTUN][64];
    char veri[MAX_SATIR][MAX_SUTUN][512];
} sonuc;

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

static int sorgu_callback(void *v, int sutun_sayi, char **degerler,
                            char **sutun_isimleri) {
    (void)v;
    if (sonuc.satir_sayi == 0) {
        sonuc.sutun_sayi = sutun_sayi < MAX_SUTUN ? sutun_sayi : MAX_SUTUN;
        for (int i = 0; i < sonuc.sutun_sayi; i++) {
            strncpy(sonuc.sutun_isim[i], sutun_isimleri[i], 63);
        }
    }
    if (sonuc.satir_sayi >= MAX_SATIR) return 0;
    int s = sonuc.satir_sayi;
    for (int i = 0; i < sutun_sayi && i < MAX_SUTUN; i++) {
        if (degerler[i]) strncpy(sonuc.veri[s][i], degerler[i], 511);
        else sonuc.veri[s][i][0] = '\0';
    }
    sonuc.satir_sayi++;
    return 0;
}

int ctr_veritabani_sorgu(const char *sql) {
    if (!db) return 0;
    sonuc.satir_sayi = 0;
    sonuc.sutun_sayi = 0;
    char *hata = NULL;
    int rc = sqlite3_exec(db, sql, sorgu_callback, NULL, &hata);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB] %s\n", hata);
        sqlite3_free(hata);
        return 0;
    }
    return sonuc.satir_sayi;
}

int ctr_veritabani_satir_sayi(void) { return sonuc.satir_sayi; }

const char* ctr_veritabani_alan(int satir, const char *alan) {
    if (satir < 0 || satir >= sonuc.satir_sayi) return "";
    for (int i = 0; i < sonuc.sutun_sayi; i++) {
        if (strcmp(sonuc.sutun_isim[i], alan) == 0)
            return sonuc.veri[satir][i];
    }
    return "";
}

const char* ctr_veritabani_alan_indeks(int satir, int sutun) {
    if (satir < 0 || satir >= sonuc.satir_sayi) return "";
    if (sutun < 0 || sutun >= sonuc.sutun_sayi) return "";
    return sonuc.veri[satir][sutun];
}
