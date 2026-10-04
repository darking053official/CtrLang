#include "ctr_calisma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Basit HTTP sunucu (Mongoose olmadan, socket ile) */

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define KAPAT(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define KAPAT(s) close(s)
    #define SOCKET int
    #define INVALID_SOCKET -1
#endif

#define MAX_SAYFA 64
#define MAX_CEVAP 65536

typedef struct {
    char yol[256];
    char yontem[16];
    ctr_sayfa_fonksiyonu fonk;
} SayfaKayit;

static SayfaKayit sayfalar[MAX_SAYFA];
static int sayfa_sayi = 0;
static int sunucu_port = 0;
static int dinleme_soketi = -1;

/* Cevap tamponu */
static char cevap_tamponu[MAX_CEVAP];
static int cevap_uzunluk = 0;

void ctr_baslat(void) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
}

void ctr_bitir(void) {
    if (dinleme_soketi >= 0) KAPAT(dinleme_soketi);
#ifdef _WIN32
    WSACleanup();
#endif
}

/* Sayı → metin (thread-safe değil, basit) */
const char* ctr_sayi_metin(double sayi) {
    static char buf[64];
    if (sayi == (int)sayi)
        snprintf(buf, sizeof(buf), "%d", (int)sayi);
    else
        snprintf(buf, sizeof(buf), "%g", sayi);
    return buf;
}

void ctr_yazdir(const char *metin) {
    printf("%s\n", metin);
}

void ctr_yazdir_sayi(double sayi) {
    printf("%s\n", ctr_sayi_metin(sayi));
}

/* Sayfa kayıt */
void ctr_sayfa_ekle(const char *yol, const char *yontem,
                     ctr_sayfa_fonksiyonu fonk) {
    if (sayfa_sayi >= MAX_SAYFA) return;
    strncpy(sayfalar[sayfa_sayi].yol, yol, 255);
    strncpy(sayfalar[sayfa_sayi].yontem, yontem, 15);
    sayfalar[sayfa_sayi].fonk = fonk;
    sayfa_sayi++;
}

/* Cevap tamponu */
static void cevap_temizle(void) {
    cevap_uzunluk = 0;
    cevap_tamponu[0] = '\0';
}

static void cevap_ekle(const char *metin) {
    int uzunluk = strlen(metin);
    if (cevap_uzunluk + uzunluk >= MAX_CEVAP) return;
    memcpy(cevap_tamponu + cevap_uzunluk, metin, uzunluk);
    cevap_uzunluk += uzunluk;
    cevap_tamponu[cevap_uzunluk] = '\0';
}

/* HTML */
void ctr_html_baslat(ctr_istek *istek) {
    (void)istek;
    cevap_temizle();
    cevap_ekle("<!DOCTYPE html>\n<html>\n<body>\n");
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
    cevap_ekle(metin);
}

void ctr_yonlendir(ctr_istek *istek, const char *yol) {
    (void)istek;
    cevap_temizle();
    cevap_ekle("HTTP/1.1 302 Found\r\nLocation: ");
    cevap_ekle(yol);
    cevap_ekle("\r\n\r\n");
}

void ctr_durum(ctr_istek *istek, int kod) {
    (void)istek;
    (void)kod;
    /* Şimdilik yoksay */
}

void ctr_veritabani_baglan(const char *dosya) {
    printf("[Veritabanı] Bağlanılacak: %s\n", dosya);
}

/* HTTP istek işle */
static void istek_isle(SOCKET istemci) {
    char tampon[8192];
    int n = recv(istemci, tampon, sizeof(tampon) - 1, 0);
    if (n <= 0) return;
    tampon[n] = '\0';
    
    /* İlk satır: GET /yol HTTP/1.1 */
    char yontem[16] = {0}, yol[512] = {0};
    sscanf(tampon, "%15s %511s", yontem, yol);
    
    /* Cevap hazırla */
    cevap_temizle();
    
    ctr_istek istek;
    istek.yol = yol;
    istek.yontem = yontem;
    istek.govde = NULL;
    istek.ic = NULL;
    
    /* Sayfa bul */
    int bulundu = 0;
    for (int i = 0; i < sayfa_sayi; i++) {
        if (strcmp(sayfalar[i].yol, yol) == 0 &&
            strcmp(sayfalar[i].yontem, yontem) == 0) {
            sayfalar[i].fonk(&istek);
            bulundu = 1;
            break;
        }
    }
    
    if (!bulundu) {
        /* Metin cevap mı, HTML cevap mı? */
        if (cevap_uzunluk == 0) {
            cevap_ekle("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n");
            cevap_ekle("404 - Bulunamadı\n");
            cevap_ekle(yol);
        } else {
            /* HTML başlatılmadıysa metin cevap */
            char gecici[65536];
            snprintf(gecici, sizeof(gecici),
                     "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n%s",
                     cevap_tamponu);
            cevap_temizle();
            cevap_ekle(gecici);
        }
    } else {
        /* Bulundu, HTML cevap */
        if (cevap_uzunluk > 0 && 
            strncmp(cevap_tamponu, "HTTP/", 5) != 0) {
            char gecici[65536];
            snprintf(gecici, sizeof(gecici),
                     "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n%s",
                     cevap_tamponu);
            cevap_temizle();
            cevap_ekle(gecici);
        }
    }
    
    send(istemci, cevap_tamponu, cevap_uzunluk, 0);
}

void ctr_sunucu_baslat(double port) {
    sunucu_port = (int)port;
    
    dinleme_soketi = socket(AF_INET, SOCK_STREAM, 0);
    if (dinleme_soketi == INVALID_SOCKET) {
        fprintf(stderr, "Socket oluşturulamadı\n");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(dinleme_soketi, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&opt, sizeof(opt));
    
    struct sockaddr_in adres;
    memset(&adres, 0, sizeof(adres));
    adres.sin_family = AF_INET;
    adres.sin_addr.s_addr = INADDR_ANY;
    adres.sin_port = htons(sunucu_port);
    
    if (bind(dinleme_soketi, (struct sockaddr*)&adres, sizeof(adres)) < 0) {
        fprintf(stderr, "Port %d kullanımda\n", sunucu_port);
        exit(1);
    }
    
    if (listen(dinleme_soketi, 10) < 0) {
        fprintf(stderr, "Dinleme başarısız\n");
        exit(1);
    }
    
    printf("\n=== CtrLang Sunucu ===\n");
    printf("http://localhost:%d\n\n", sunucu_port);
}

void ctr_sunucu_dongu(void) {
    for (;;) {
        struct sockaddr_in istemci_adres;
        socklen_t istemci_uzunluk = sizeof(istemci_adres);
        
        SOCKET istemci = accept(dinleme_soketi,
                                 (struct sockaddr*)&istemci_adres,
                                 &istemci_uzunluk);
        if (istemci == INVALID_SOCKET) continue;
        
        istek_isle(istemci);
        KAPAT(istemci);
    }
}