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

/* Başlat / Bitir */
void ctr_baslat(void);
void ctr_bitir(void);

/* Yazdır */
void ctr_yazdir(const char *metin);
void ctr_yazdir_sayi(double sayi);

/* Sayı → metin */
const char* ctr_sayi_metin(double sayi);

/* Ortam değişkeninden port oku */
double ctr_env_port(const char *isim, double varsayilan);

/* Sayfa kayıt */
void ctr_sayfa_ekle(const char *yol, const char *yontem,
                     ctr_sayfa_fonksiyonu fonk);

/* Sunucu (host + port) */
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

/* Veritabanı */
void ctr_veritabani_baglan(const char *dosya);
void ctr_veritabani_calistir(const char *sql);
int  ctr_veritabani_sorgu(const char *sql);
int  ctr_veritabani_satir_sayi(void);
const char* ctr_veritabani_alan(int satir, const char *alan);
const char* ctr_veritabani_alan_indeks(int satir, int sutun);

/* İstek */
const char* ctr_istek_form(ctr_istek *istek, const char *isim);
const char* ctr_istek_sorgu(ctr_istek *istek, const char *isim);
const char* ctr_istek_tip(ctr_istek *istek);

#endif
