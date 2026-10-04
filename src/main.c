#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "platform.h"
#include "hata.h"
#include "bellek.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "uret.h"

/* ============ GLOBAL ============ */
int g_port = 0;
char g_port_env[128] = {0};
char g_host[128] = {0};
int g_debug = 0;
int g_verbose = 0;
int g_quiet = 0;

/* ============ DOSYA OKU ============ */
static char* dosya_oku(const char *yol) {
    FILE *f = fopen(yol, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long boyut = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *icerik = malloc(boyut + 1);
    if (!icerik) { fclose(f); return NULL; }
    size_t okunan = fread(icerik, 1, boyut, f);
    icerik[okunan] = '\0';
    fclose(f);
    return icerik;
}

/* ============ YARDIM ============ */
static void kullanim(const char *prog) {
    printf("CtrLang %s\n", CTR_SURUM);
    printf("Kullanım: %s [komut] [seçenekler] <dosya.ctr>\n\n", prog);

    printf("Komutlar:\n");
    printf("  run <dosya>        Derle ve çalıştır\n");
    printf("  build <dosya>      C'ye çevir ve derle\n");
    printf("  check <dosya>      Sözdizimi kontrol\n");
    printf("  new <proje>        Yeni proje oluştur\n");
    printf("  temizle            Üretilenleri sil\n");
    printf("  temizle --hepsi    Her şeyi sil\n");
    printf("  listele            Tüm komutlar\n");
    printf("  update             CtrLang'i güncelle\n\n");

    printf("Sunucu:\n");
    printf("  -p <port>          Port belirt (örn: -p 8080)\n");
    printf("                     Ortam: -p \"process.env.PORT\"\n");
    printf("  -h <host>          Host belirt (varsayılan: 0.0.0.0)\n\n");

    printf("Mod:\n");
    printf("  -d, --debug        Debug modu\n");
    printf("  -v, --verbose      Detaylı çıktı\n");
    printf("  -q, --quiet        Sessiz mod\n");
    printf("  --no-color         Renksiz çıktı\n\n");

    printf("Diğer:\n");
    printf("  -o <dosya>         Çıktı C dosyası\n");
    printf("  --derle            C'ye çevir ve derle\n");
    printf("  --calistir         Derle ve çalıştır\n");
    printf("  --ast              AST'yi yazdır\n");
    printf("  --token            Token'ları yazdır\n");
    printf("  --platform         Platform bilgisi\n");
    printf("  --surum            Sürüm\n");
    printf("  --yardim           Bu mesaj\n");
}

/* ============ KOMUT LİSTESİ ============ */
static void ctr_listele(void) {
    printf("\n=== CtrLang Komutları ===\n\n");

    printf("Çalıştırma:\n");
    printf("  ctr run <dosya.ctr>              Derle ve çalıştır\n");
    printf("  ctr run <dosya.ctr> -p 8080      Port belirt\n");
    printf("  ctr run <dosya.ctr> -h 0.0.0.0   Host belirt\n");
    printf("  ctr run <dosya.ctr> -d           Debug modu\n");
    printf("  ctr run <dosya.ctr> -v           Detaylı çıktı\n");
    printf("  ctr build <dosya.ctr>            C'ye çevir ve derle\n");
    printf("  ctr check <dosya.ctr>            Sözdizimi kontrol\n");
    printf("  ctr <dosya.ctr>                  Sadece C kodu üret\n\n");

    printf("Proje:\n");
    printf("  ctr new <proje>                  Yeni proje oluştur\n");
    printf("  ctr temizle                      Üretilen dosyaları sil\n");
    printf("  ctr temizle --hepsi              Her şeyi sil\n\n");

    printf("Debug:\n");
    printf("  ctr --token <dosya.ctr>          Token'ları göster\n");
    printf("  ctr --ast <dosya.ctr>            AST'yi göster\n\n");

    printf("Bilgi:\n");
    printf("  ctr --platform                   Platform bilgisi\n");
    printf("  ctr --surum                      Sürüm\n");
    printf("  ctr --yardim                     Yardım\n");
    printf("  ctr listele                      Bu mesaj\n\n");

    printf("Güncelleme:\n");
    printf("  ctr update                       CtrLang'i güncelle\n\n");

    printf("Örnekler:\n");
    printf("  ctr run app.ctr -p 3000\n");
    printf("  ctr run app.ctr -p \"process.env.PORT\" -h 0.0.0.0\n");
    printf("  ctr run app.ctr -p 8080 -d -v\n\n");
}

/* ============ TOKEN MODU ============ */
static void token_modu(const char *kaynak) {
    Lexer lx;
    lexer_baslat(&lx, kaynak);
    Token t;
    do {
        t = lexer_sonraki(&lx);
        printf("%d:%d  %-15s  ", t.satir, t.sutun, token_tip_adi(t.tip));
        if (t.tip == TOKEN_METIN || t.tip == TOKEN_ISIM) {
            printf("\"%.*s\"", (int)t.uzunluk, t.baslangic);
        } else if (t.tip == TOKEN_SAYI) {
            printf("%g", t.sayi_deger);
        }
        printf("\n");
    } while (t.tip != TOKEN_EOF);
}

/* ============ DERLEME ============ */
static int gcc_derle(const char *c_dosya, const char *cikti) {
    char komut[4096];
    const char *ana = getenv("HOME");
    if (!ana) ana = ".";

    snprintf(komut, sizeof(komut),
        "cc -std=c11 -O2 -w "
        "-Ikutuphane -Ivendor/mongoose -Ivendor/sqlite "
        "-Ivendor/cjson -Ivendor/sds -Ivendor/uthash "
        "-o %s %s "
        "%s/gecici/ctr_calisma.o %s/gecici/ctr_veritabani.o %s/gecici/ctr_json.o "
        "%s/gecici/sqlite3.o %s/gecici/mongoose.o %s/gecici/cJSON.o %s/gecici/sds.o "
        "-lpthread -ldl -lm",
        cikti, c_dosya,
        ana, ana, ana,
        ana, ana, ana, ana);

    if (g_verbose) printf("Komut: %s\n", komut);
    if (!g_quiet) printf("Derleniyor...\n");

    return system(komut);
}

/* ============ GÜNCELLE ============ */
static void ctr_guncelle(void) {
    char dizin[512];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(dizin, sizeof(dizin), "%s/CtrLang", home);

    char kontrol[600];
    snprintf(kontrol, sizeof(kontrol), "%s/.git", dizin);

#ifndef _WIN32
    if (access(kontrol, F_OK) != 0) {
        fprintf(stderr, "\n[HATA] CtrLang dizini bulunamadı: %s\n", dizin);
        fprintf(stderr, "Yeniden kurun:\n");
        fprintf(stderr, "  git clone https://github.com/darking053official/CtrLang.git\n\n");
        exit(1);
    }
#endif

    if (!g_quiet) {
        printf("\n=== CtrLang Güncelleme ===\n\n");
        printf("Dizin: %s\n\n", dizin);
        printf("-> git pull...\n");
    }

    char komut[2048];
    snprintf(komut, sizeof(komut), "cd \"%s\" && git pull", dizin);
    if (system(komut) != 0) {
        fprintf(stderr, "\n[HATA] git pull başarısız\n\n");
        exit(1);
    }

    if (!g_quiet) printf("\n-> Yeniden derleniyor...\n");

#ifdef _WIN32
    snprintf(komut, sizeof(komut), "cd \"%s\" && kur.bat", dizin);
#else
    snprintf(komut, sizeof(komut), "cd \"%s\" && chmod +x kur.sh && ./kur.sh", dizin);
#endif
    if (system(komut) != 0) {
        fprintf(stderr, "\n[HATA] Derleme başarısız\n\n");
        exit(1);
    }

    if (!g_quiet) printf("\n=== Güncelleme tamam! ===\n\n");
}

/* ============ YENİ PROJE ============ */
static void ctr_yeni_proje(const char *isim) {
    char komut[1024];
    if (!g_quiet) {
        printf("\n=== Yeni CtrLang Projesi ===\n\n");
        printf("Proje: %s\n\n", isim);
    }

#ifdef _WIN32
    snprintf(komut, sizeof(komut), "mkdir \"%s\" 2>nul", isim);
#else
    snprintf(komut, sizeof(komut), "mkdir -p \"%s\"", isim);
#endif
    system(komut);

    char yol[512];
    snprintf(yol, sizeof(yol), "%s/ana.ctr", isim);

    FILE *f = fopen(yol, "w");
    if (!f) {
        fprintf(stderr, "[HATA] Dosya oluşturulamadı: %s\n", yol);
        exit(1);
    }
    fprintf(f,
        "# %s - CtrLang projesi\n"
        "# Oluşturuldu: ctr new\n\n"
        "sunucu başlat(3000)\n\n"
        "sayfa \"/\" {\n"
        "    html {\n"
        "        başlık \"%s\"\n"
        "        paragraf \"CtrLang ile yazıldı\"\n"
        "    }\n"
        "}\n",
        isim, isim);
    fclose(f);

    snprintf(yol, sizeof(yol), "%s/README.md", isim);
    f = fopen(yol, "w");
    if (f) {
        fprintf(f,
            "# %s\n\n"
            "CtrLang projesi.\n\n"
            "## Çalıştırma\n\n"
            "```bash\n"
            "ctr run ana.ctr\n"
            "```\n\n"
            "Tarayıcıda: http://localhost:3000\n",
            isim);
        fclose(f);
    }

    if (!g_quiet) {
        printf("Proje oluşturuldu: %s/\n", isim);
        printf("  %s/ana.ctr\n", isim);
        printf("  %s/README.md\n\n", isim);
        printf("Çalıştır:\n");
        printf("  cd %s\n", isim);
        printf("  ctr run ana.ctr\n\n");
    }
}

/* ============ TEMİZLE ============ */
static void ctr_temizle(int hepsi) {
    if (!g_quiet) printf("\n=== Temizlik ===\n\n");

    if (hepsi) {
        if (!g_quiet) printf("-> build/ siliniyor...\n");
#ifdef _WIN32
        system("rmdir /s /q build 2>nul");
        system("del /q ctrc.exe 2>nul");
#else
        system("rm -rf build");
        system("rm -f ctrc");
#endif
    }

    if (!g_quiet) printf("-> Üretilen .c dosyaları siliniyor...\n");
#ifdef _WIN32
    system("del /q ornekler\\*.ctr.c 2>nul");
    system("del /q *.ctr.c 2>nul");
    system("del /q ornekler\\merhaba.exe 2>nul");
    system("del /q ornekler\\sayfa.exe 2>nul");
#else
    system("rm -f *.ctr.c ornekler/*.ctr.c");
    system("rm -f ornekler/merhaba ornekler/sayfa 2>/dev/null");
#endif

    if (!g_quiet) printf("\nTemizlendi\n\n");
}

/* ============ MAIN ============ */
int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    if (argc < 2) {
        kullanim(argv[0]);
        return 1;
    }

    /* --- Kısayol komutlar --- */
    if (strcmp(argv[1], "run") == 0 && argc >= 3) {
        int yeni_argc = argc - 1;
        char **yeni = malloc(sizeof(char*) * (yeni_argc + 2));
        yeni[0] = argv[0];
        yeni[1] = "--calistir";
        for (int i = 2; i < argc; i++) yeni[i] = argv[i];
        yeni[yeni_argc + 1] = NULL;
        int rc = main(yeni_argc + 1, yeni);
        free(yeni);
        return rc;
    }
    if (strcmp(argv[1], "build") == 0 && argc >= 3) {
        int yeni_argc = argc - 1;
        char **yeni = malloc(sizeof(char*) * (yeni_argc + 2));
        yeni[0] = argv[0];
        yeni[1] = "--derle";
        for (int i = 2; i < argc; i++) yeni[i] = argv[i];
        yeni[yeni_argc + 1] = NULL;
        int rc = main(yeni_argc + 1, yeni);
        free(yeni);
        return rc;
    }
    if (strcmp(argv[1], "check") == 0 && argc >= 3) {
        int yeni_argc = argc - 1;
        char **yeni = malloc(sizeof(char*) * (yeni_argc + 2));
        yeni[0] = argv[0];
        yeni[1] = "--ast";
        for (int i = 2; i < argc; i++) yeni[i] = argv[i];
        yeni[yeni_argc + 1] = NULL;
        int rc = main(yeni_argc + 1, yeni);
        free(yeni);
        return rc;
    }
    if (strcmp(argv[1], "new") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Kullanım: ctr new <proje_adi>\n");
            return 1;
        }
        ctr_yeni_proje(argv[2]);
        return 0;
    }
    if (strcmp(argv[1], "temizle") == 0) {
        int hepsi = (argc >= 3 && strcmp(argv[2], "--hepsi") == 0);
        ctr_temizle(hepsi);
        return 0;
    }
    if (strcmp(argv[1], "listele") == 0) { ctr_listele(); return 0; }
    if (strcmp(argv[1], "yardim") == 0) { kullanim(argv[0]); return 0; }
    if (strcmp(argv[1], "surum") == 0) { printf("CtrLang %s\n", CTR_SURUM); return 0; }
    if (strcmp(argv[1], "update") == 0 ||
        strcmp(argv[1], "guncelle") == 0) { ctr_guncelle(); return 0; }

    /* --- İlk geçiş: modları al --- */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 ||
            strcmp(argv[i], "--debug") == 0) g_debug = 1;
        if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--verbose") == 0) g_verbose = 1;
        if (strcmp(argv[i], "-q") == 0 ||
            strcmp(argv[i], "--quiet") == 0) g_quiet = 1;
    }

    /* --- Seçenekler --- */
    const char *girdi = NULL;
    const char *cikti = NULL;
    int ast_modu = 0;
    int token_modu_flag = 0;
    int derle_flag = 0;
    int calistir_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--yardim") == 0) { kullanim(argv[0]); return 0; }
        if (strcmp(argv[i], "--surum") == 0) { printf("CtrLang %s\n", CTR_SURUM); return 0; }
        if (strcmp(argv[i], "--platform") == 0) { ctr_platform_yazdir(); return 0; }
        if (strcmp(argv[i], "--ast") == 0) { ast_modu = 1; continue; }
        if (strcmp(argv[i], "--token") == 0) { token_modu_flag = 1; continue; }
        if (strcmp(argv[i], "--derle") == 0) { derle_flag = 1; continue; }
        if (strcmp(argv[i], "--calistir") == 0) { derle_flag = 1; calistir_flag = 1; continue; }

        /* Modlar */
        if (strcmp(argv[i], "-d") == 0 ||
            strcmp(argv[i], "--debug") == 0) continue;
        if (strcmp(argv[i], "-v") == 0 ||
            strcmp(argv[i], "--verbose") == 0) continue;
        if (strcmp(argv[i], "-q") == 0 ||
            strcmp(argv[i], "--quiet") == 0) continue;
        if (strcmp(argv[i], "--no-color") == 0) continue;

        /* Port */
        if ((strcmp(argv[i], "-p") == 0 ||
             strcmp(argv[i], "--port") == 0) && i + 1 < argc) {
            const char *deger = argv[++i];
            if (strncmp(deger, "process.env.", 12) == 0) {
                strncpy(g_port_env, deger + 12, sizeof(g_port_env) - 1);
                g_port_env[sizeof(g_port_env) - 1] = '\0';
                if (!g_quiet) printf("Port: ortam değişkeni %s\n", g_port_env);
            } else {
                g_port = atoi(deger);
                if (g_port <= 0 || g_port > 65535) {
                    fprintf(stderr, "Geçersiz port: %s\n", deger);
                    return 1;
                }
                if (!g_quiet) printf("Port: %d\n", g_port);
            }
            continue;
        }

        /* Host */
        if ((strcmp(argv[i], "-h") == 0 ||
             strcmp(argv[i], "--host") == 0) && i + 1 < argc) {
            strncpy(g_host, argv[++i], sizeof(g_host) - 1);
            g_host[sizeof(g_host) - 1] = '\0';
            if (!g_quiet) printf("Host: %s\n", g_host);
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            cikti = argv[++i];
            continue;
        }

        girdi = argv[i];
    }

    if (!girdi) {
        kullanim(argv[0]);
        return 1;
    }

    /* Debug modu bilgisi */
    if (g_debug) {
        fprintf(stderr, "[DEBUG] Dosya: %s\n", girdi);
        if (g_port > 0) fprintf(stderr, "[DEBUG] Port: %d\n", g_port);
        if (g_port_env[0]) fprintf(stderr, "[DEBUG] Port env: %s\n", g_port_env);
        if (g_host[0]) fprintf(stderr, "[DEBUG] Host: %s\n", g_host);
    }

    char *kaynak = dosya_oku(girdi);
    if (!kaynak) {
        ctr_hata_basit(HATA_DOSYA, "Dosya açılamadı");
    }

    if (token_modu_flag) {
        token_modu(kaynak);
        free(kaynak);
        return 0;
    }

    Arena *arena = arena_olustur(0);
    Parser p;
    parser_baslat(&p, kaynak, arena);
    ASTDugum *kok = parser_calistir(&p);

    if (ast_modu) {
        printf("=== AST ===\n");
        ast_yazdir(kok, 0);
        free(kaynak);
        arena_yok_et(arena);
        return 0;
    }

    char varsayilan_c[512];
    if (!cikti) {
        snprintf(varsayilan_c, sizeof(varsayilan_c), "%s.c", girdi);
        cikti = varsayilan_c;
    }

    FILE *f = fopen(cikti, "w");
    if (!f) {
        ctr_hata_basit(HATA_DOSYA, "Çıktı dosyası açılamadı");
    }

    uret_baslat(f);
    uret_program(f, kok);
    uret_bitir(f);
    fclose(f);

    if (!g_quiet) printf("Üretildi: %s\n", cikti);

    if (derle_flag) {
        char cikti_bin[512];
        strncpy(cikti_bin, girdi, sizeof(cikti_bin) - 1);
        cikti_bin[sizeof(cikti_bin) - 1] = '\0';
        char *nokta = strrchr(cikti_bin, '.');
        if (nokta) *nokta = '\0';

        if (gcc_derle(cikti, cikti_bin) != 0) {
            ctr_hata_basit(HATA_URET, "Derleme başarısız");
        }

        if (!g_quiet) printf("Derlendi: %s\n", cikti_bin);

        if (calistir_flag) {
            char komut[512];
            snprintf(komut, sizeof(komut), "./%s", cikti_bin);
            system(komut);
        }
    }

    free(kaynak);
    arena_yok_et(arena);
    return 0;
}
