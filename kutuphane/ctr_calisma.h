#ifndef CTR_CALISMA_H
#define CTR_CALISMA_H

#include <stddef.h>

typedef struct ctr_istek {
    char yol[512];
    char yontem[16];
    char govde[8192];
    size_t govde_uzunluk;
    void *ic;

    struct { char isim[64]; char deger[512]; } form[32];
    int form_sayi;

    struct { char isim[64]; char deger[512]; } sorgu[32];
    int sorgu_sayi;
} ctr_istek;

typedef void (*ctr_sayfa_fonksiyonu)(ctr_istek *istek);

/* Liste */
typedef struct {
    int sayi;
    int kapasite;
    char **elemanlar;
} ctr_liste;

/* Başlat / Bitir */
void ctr_baslat(void);
void ctr_bitir(void);

/* Yazdır */
void ctr_yazdir(const char *metin);
void ctr_yazdir_sayi(double sayi);
const char* ctr_sayi_metin(double sayi);

/* Ortam portu */
double ctr_env_port(const char *isim, double varsayilan);

/* Sayfa */
void ctr_sayfa_ekle(const char *yol, const char *yontem,
                     ctr_sayfa_fonksiyonu fonk);

/* Sunucu */
void ctr_sunucu_baslat(double port, const char *host);
void ctr_sunucu_dongu(void);

/* HTML */
void ctr_html_baslat(ctr_istek *istek);
void ctr_html_bitir(ctr_istek *istek);
void ctr_html_ac(ctr_istek *istek, const char *etiket);
void ctr_html_kapat(ctr_istek *istek, const char *etiket);
void ctr_html_metin(ctr_istek *istek, const char *metin);

/* Yönlendir / Durum */
void ctr_yonlendir(ctr_istek *istek, const char *yol);
void ctr_durum(ctr_istek *istek, int kod);

/* JSON */
void ctr_json_baslat(ctr_istek *istek);
void ctr_json_bitir(ctr_istek *istek);
void ctr_json_ekle(ctr_istek *istek, const char *anahtar, const char *deger);
void ctr_json_ekle_sayi(ctr_istek *istek, const char *anahtar, double deger);

/* İstek */
const char* ctr_istek_form(ctr_istek *istek, const char *isim);
const char* ctr_istek_sorgu(ctr_istek *istek, const char *isim);
const char* ctr_istek_tip(ctr_istek *istek);

/* Liste fonksiyonları */
ctr_liste* ctr_liste_olustur(void);
void ctr_liste_ekle(ctr_liste *l, const char *eleman);
const char* ctr_liste_al(ctr_liste *l, int indeks);
void ctr_liste_sil(ctr_liste *l);

/* Veritabanı (imzalar ctr_veritabani.h ile aynı olmalı) */
void ctr_veritabani_baglan(const char *dosya);
void ctr_veritabani_kapat(void);
void ctr_veritabani_calistir(const char *sql);
ctr_liste* ctr_veritabani_sorgu(const char *sql);

#endif
