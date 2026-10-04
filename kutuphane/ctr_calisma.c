#include "ctr_calisma.h"
#include "ctr_veritabani.h"
#include "ctr_json.h"
#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SAYFA 64
#define MAX_CEVAP (1024 * 1024)

typedef struct {
    char yol[256];
    char yontem[16];
    ctr_sayfa_fonksiyonu fonk;
} SayfaKayit;

static SayfaKayit sayfalar[MAX_SAYFA];
static int sayfa_sayi = 0;
static int sunucu_port = 8080;

/* Cevap tamponu */
static char cevap_tamponu[MAX_CEVAP];
static size_t cevap_uzunluk = 0;
static int cevap_durum = 200;
static char cevap_yonlendir[512] = {0};
static char cevap_tip[128] = "text/html; charset=utf-8";

/* JSON tamponu */
static char json_tamponu[65536];
static size_t json_uzunluk = 0;

/* ============ BAŞLAT / BİTİR ============ */

void ctr_baslat(void) {
    /* Boş - ileride global init */
}

void ctr_bitir(void) {
    ctr_veritabani_kapat();
}

/* ============ SAYI → METİN ============ */

const char* ctr_sayi_metin(double sayi) {
    static char buf[64];
    if (sayi == (int)sayi)
        snprintf(buf, sizeof(buf), "%d", (int)sayi);
    else
        snprintf(buf, sizeof(buf), "%g", sayi);
    return buf;
}

/* ============ CEVAP TAMPONU ============ */

static void cevap_temizle(void) {
    cevap_uzunluk = 0;
    cevap_tamponu[0] = '\0';
    cevap_durum = 200;
    cevap_yonlendir[0] = '\0';
    strcpy(cevap_tip, "text/html; charset=utf-8");
}

static void cevap_ekle(const char *metin) {
    if (!metin) return;
    size_t uzunluk = strlen(metin);
    if (cevap_uzunluk + uzunluk >= MAX_CEVAP - 1) return;
    memcpy(cevap_tamponu + cevap_uzunluk, metin, uzunluk);
    cevap_uzunluk += uzunluk;
    cevap_tamponu[cevap_uzunluk] = '\0';
}

static void cevap_ekle_n(const char *metin, size_t uzunluk) {
    if (!metin || uzunluk == 0) return;
    if (cevap_uzunluk + uzunluk >= MAX_CEVAP - 1) return;
    memcpy(cevap_tamponu + cevap_uzunluk, metin, uzunluk);
    cevap_uzunluk += uzunluk;
    cevap_tamponu[cevap_uzunluk] = '\0';
}

/* ============ YAZDIR ============ */

void ctr_yazdir(const char *metin) {
    if (!metin) return;
    cevap_ekle(metin);
    cevap_ekle("\n");
}

void ctr_yazdir_sayi(double sayi) {
    ctr_yazdir(ctr_sayi_metin(sayi));
}

/* ============ SAYFA KAYIT ============ */

void ctr_sayfa_ekle(const char *yol, const char *yontem,
                     ctr_sayfa_fonksiyonu fonk) {
    if (sayfa_sayi >= MAX_SAYFA) {
        fprintf(stderr, "Maksimum sayfa sayısı aşıldı\n");
        return;
    }
    strncpy(sayfalar[sayfa_sayi].yol, yol, 255);
    sayfalar[sayfa_sayi].yol[255] = '\0';
    strncpy(sayfalar[sayfa_sayi].yontem, yontem ? yontem : "GET", 15);
    sayfalar[sayfa_sayi].yontem[15] = '\0';
    sayfalar[sayfa_sayi].fonk = fonk;
    sayfa_sayi++;
}

/* ============ HTML ============ */

void ctr_html_baslat(ctr_istek *istek) {
    (void)istek;
    cevap_temizle();
    cevap_ekle("<!DOCTYPE html>\n<html lang=\"tr\">\n<head>\n");
    cevap_ekle("<meta charset=\"utf-8\">\n");
    cevap_ekle("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
    cevap_ekle("</head>\n<body>\n");
}

void ctr_html_bitir(ctr_istek *istek) {
    (void)istek;
    cevap_ekle("</body>\n</html>\n");
}

void ctr_html_ac(ctr_istek *istek, const char *etiket) {
    (void)istek;
    cevap_ekle("<");
    cevap_ekle(etiket);
    cevap_ekle(">");
}

void ctr_html_kapat(ctr_istek *istek, const char *etiket) {
    (void)istek;
    cevap_ekle("</");
    cevap_ekle(etiket);
    cevap_ekle(">\n");
}

void ctr_html_metin(ctr_istek *istek, const char *metin) {
    (void)istek;
    if (metin) cevap_ekle(metin);
}

/* ============ JSON ============ */

void ctr_json_baslat(ctr_istek *istek) {
    (void)istek;
    cevap_temizle();
    strcpy(cevap_tip, "application/json; charset=utf-8");
    json_uzunluk = 0;
    json_tamponu[0] = '\0';
    cevap_ekle("{");
}

void ctr_json_bitir(ctr_istek *istek) {
    (void)istek;
    cevap_ekle("}");
}

void ctr_json_ekle(ctr_istek *istek, const char *anahtar, const char *deger) {
    (void)istek;
    if (cevap_uzunluk > 2) cevap_ekle(",");
    cevap_ekle("\"");
    cevap_ekle(anahtar);
    cevap_ekle("\":\"");
    
    /* Kaçış karakterleri */
    for (const char *p = deger; *p; p++) {
        if (*p == '"' || *p == '\\') {
            char buf[3] = { '\\', *p, 0 };
            cevap_ekle(buf);
        } else if (*p == '\n') {
            cevap_ekle("\\n");
        } else {
            char buf[2] = { *p, 0 };
            cevap_ekle(buf);
        }
    }
    
    cevap_ekle("\"");
}

void ctr_json_ekle_sayi(ctr_istek *istek, const char *anahtar, double deger) {
    (void)istek;
    if (cevap_uzunluk > 2) cevap_ekle(",");
    cevap_ekle("\"");
    cevap_ekle(anahtar);
    cevap_ekle("\":");
    cevap_ekle(ctr_sayi_metin(deger));
}

/* ============ YÖNLENDİR / DURUM ============ */

void ctr_yonlendir(ctr_istek *istek, const char *yol) {
    (void)istek;
    cevap_durum = 302;
    strncpy(cevap_yonlendir, yol, 511);
    cevap_yonlendir[511] = '\0';
}

void ctr_durum(ctr_istek *istek, int kod) {
    (void)istek;
    cevap_durum = kod;
}

/* ============ İSTEK ============ */

const char* ctr_istek_form(ctr_istek *istek, const char *isim) {
    if (!istek) return "";
    for (int i = 0; i < istek->form_sayi; i++) {
        if (strcmp(istek->form[i].isim, isim) == 0)
            return istek->form[i].deger;
    }
    return "";
}

const char* ctr_istek_sorgu(ctr_istek *istek, const char *isim) {
    if (!istek) return "";
    for (int i = 0; i < istek->sorgu_sayi; i++) {
        if (strcmp(istek->sorgu[i].isim, isim) == 0)
            return istek->sorgu[i].deger;
    }
    return "";
}

const char* ctr_istek_tip(ctr_istek *istek) {
    if (!istek) return "";
    return istek->yontem;
}

/* ============ URL DECODE ============ */

static void url_decode(char *cikti, const char *girdi, size_t max) {
    size_t j = 0;
    for (size_t i = 0; girdi[i] && j < max - 1; i++) {
        if (girdi[i] == '%' && girdi[i+1] && girdi[i+2]) {
            char hex[3] = { girdi[i+1], girdi[i+2], 0 };
            cikti[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (girdi[i] == '+') {
            cikti[j++] = ' ';
        } else {
            cikti[j++] = girdi[i];
        }
    }
    cikti[j] = '\0';
}

/* ============ FORM ÇÖZ ============ */

static void form_coz(ctr_istek *istek, const char *veri) {
    if (!veri || !*veri) return;
    
    char gecici[8192];
    strncpy(gecici, veri, sizeof(gecici) - 1);
    gecici[sizeof(gecici) - 1] = '\0';
    
    char *p = gecici;
    while (*p && istek->form_sayi < 32) {
        char *esit = strchr(p, '=');
        if (!esit) break;
        char *amp = strchr(p, '&');
        
        *esit = '\0';
        if (amp) *amp = '\0';
        
        url_decode(istek->form[istek->form_sayi].isim, p, 64);
        url_decode(istek->form[istek->form_sayi].deger, esit + 1, 512);
        istek->form_sayi++;
        
        if (!amp) break;
        p = amp + 1;
    }
}

/* ============ SORGU ÇÖZ ============ */

static void sorgu_coz(ctr_istek *istek, const char *veri) {
    if (!veri || !*veri) return;
    
    char gecici[2048];
    strncpy(gecici, veri, sizeof(gecici) - 1);
    gecici[sizeof(gecici) - 1] = '\0';
    
    char *p = gecici;
    while (*p && istek->sorgu_sayi < 32) {
        char *esit = strchr(p, '=');
        if (!esit) break;
        char *amp = strchr(p, '&');
        
        *esit = '\0';
        if (amp) *amp = '\0';
        
        url_decode(istek->sorgu[istek->sorgu_sayi].isim, p, 64);
        url_decode(istek->sorgu[istek->sorgu_sayi].deger, esit + 1, 512);
        istek->sorgu_sayi++;
        
        if (!amp) break;
        p = amp + 1;
    }
}

/* ============ ROTA EŞLEŞTİR ============ */

/* /kullanici/{id} gibi yollar için */
static int rota_esles(const char *sablon, const char *yol, ctr_istek *istek) {
    if (strcmp(sablon, yol) == 0) return 1;
    
    const char *s = sablon;
    const char *y = yol;
    
    while (*s && *y) {
        if (*s == '{') {
            /* Parametre başlangıcı */
            s++;
            const char *bas = y;
            while (*s && *s != '}') s++;
            if (!*s) return 0;
            
            /* Değeri kaydet */
            if (istek && istek->sorgu_sayi < 32) {
                char isim[64] = {0};
                const char *sb = sablon;
                const char *se = s;
                /* Sablonda ismi bul */
                while (sb < se && *sb != '{') sb++;
                sb++;
                size_t isim_uz = 0;
                while (sb < se && *sb != '}' && isim_uz < 63) {
                    isim[isim_uz++] = *sb++;
                }
                isim[isim_uz] = '\0';
                
                /* Değeri bul */
                const char *deger_bas = bas;
                const char *deger_son = y;
                while (*deger_son && *deger_son != '/') deger_son++;
                
                strncpy(istek->sorgu[istek->sorgu_sayi].isim, isim, 63);
                size_t dl = deger_son - deger_bas;
                if (dl > 511) dl = 511;
                memcpy(istek->sorgu[istek->sorgu_sayi].deger, deger_bas, dl);
                istek->sorgu[istek->sorgu_sayi].deger[dl] = '\0';
                istek->sorgu_sayi++;
                
                y = deger_son;
            }
            s++;
        } else if (*s == *y) {
            s++;
            y++;
        } else {
            return 0;
        }
    }
    
    return *s == '\0' && *y == '\0';
}

/* ============ MONGOOSE OLAY İŞLEYİCİ ============ */

static void olay_isle(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;
    
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    
    ctr_istek istek;
    memset(&istek, 0, sizeof(istek));
    
    /* Yol */
    size_t yol_uzunluk = hm->uri.len < 511 ? hm->uri.len : 511;
    memcpy(istek.yol, hm->uri.buf, yol_uzunluk);
    istek.yol[yol_uzunluk] = '\0';
    
    /* Yöntem */
    size_t yontem_uzunluk = hm->method.len < 15 ? hm->method.len : 15;
    memcpy(istek.yontem, hm->method.buf, yontem_uzunluk);
    istek.yontem[yontem_uzunluk] = '\0';
    
    /* Gövde */
    if (hm->body.len > 0) {
        size_t govde_uzunluk = hm->body.len < 8191 ? hm->body.len : 8191;
        memcpy(istek.govde, hm->body.buf, govde_uzunluk);
        istek.govde[govde_uzunluk] = '\0';
        istek.govde_uzunluk = govde_uzunluk;
        
        if (strcmp(istek.yontem, "POST") == 0) {
            form_coz(&istek, istek.govde);
        }
    }
    
    /* Sorgu parametreleri */
    if (hm->query.len > 0) {
        char sorgu_str[2048];
        size_t su = hm->query.len < 2047 ? hm->query.len : 2047;
        memcpy(sorgu_str, hm->query.buf, su);
        sorgu_str[su] = '\0';
        sorgu_coz(&istek, sorgu_str);
    }
    
    istek.ic = c;
    
    /* Cevap hazırla */
    cevap_temizle();
    
    /* Sayfa bul */
    int bulundu = 0;
    for (int i = 0; i < sayfa_sayi; i++) {
        if (strcmp(sayfalar[i].yontem, istek.yontem) != 0) continue;
        if (rota_esles(sayfalar[i].yol, istek.yol, &istek)) {
            sayfalar[i].fonk(&istek);
            bulundu = 1;
            break;
        }
    }
    
    if (!bulundu) {
        mg_http_reply(c, 404,
            "Content-Type: text/html; charset=utf-8\r\n",
            "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>404</title></head>"
            "<body><h1>404 - Bulunamadı</h1><p>Yol: %s</p><p>Yöntem: %s</p></body></html>",
            istek.yol, istek.yontem);
        return;
    }
    
    /* Yönlendirme */
    if (cevap_durum == 302 && cevap_yonlendir[0]) {
        char basliklar[1024];
        snprintf(basliklar, sizeof(basliklar),
                 "Location: %s\r\nContent-Type: %s\r\n",
                 cevap_yonlendir, cevap_tip);
        mg_http_reply(c, 302, basliklar, "");
        return;
    }
    
    /* Normal cevap */
    char basliklar[256];
    snprintf(basliklar, sizeof(basliklar),
             "Content-Type: %s\r\n", cevap_tip);
    mg_http_reply(c, cevap_durum, basliklar, "%s", cevap_tamponu);
}

/* ============ SUNUCU ============ */

void ctr_sunucu_baslat(double port) {
    sunucu_port = (int)port;
}

void ctr_sunucu_dongu(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    char adres[64];
    snprintf(adres, sizeof(adres), "http://0.0.0.0:%d", sunucu_port);
    
    if (mg_http_listen(&mgr, adres, olay_isle, NULL) == NULL) {
        fprintf(stderr, "Port %d dinlenemedi\n", sunucu_port);
        exit(1);
    }
    
    printf("\n=== CtrLang Sunucu ===\n");
    printf("http://localhost:%d\n", sunucu_port);
    printf("Durdurmak için Ctrl+C\n\n");
    
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    mg_mgr_free(&mgr);
}
