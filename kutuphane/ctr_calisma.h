#ifndef CTR_CALISMA_H
#define CTR_CALISMA_H

/* İstek yapısı */
typedef struct {
    const char *yol;
    const char *yontem;
    const char *govde;
    void *ic;  /* İç veri (Mongoose bağlantısı) */
} ctr_istek;

/* Sayfa fonksiyonu */
typedef void (*ctr_sayfa_fonksiyonu)(ctr_istek *istek);

/* Başlat / Bitir */
void ctr_baslat(void);
void ctr_bitir(void);

/* Yazdır */
void ctr_yazdir(const char *metin);
void ctr_yazdir_sayi(double sayi);

/* Sayı → metin */
const char* ctr_sayi_metin(double sayi);

/* Sayfa kayıt */
void ctr_sayfa_ekle(const char *yol, const char *yontem,
                     ctr_sayfa_fonksiyonu fonk);

/* Sunucu */
void ctr_sunucu_baslat(double port);
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

/* Veritabanı */
void ctr_veritabani_baglan(const char *dosya);

#endif