#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
    #include <unistd.h>
#endif

#include "platform.h"
#include "hata.h"
#include "bellek.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "uret.h"

/* Dosya oku */
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

/* Yardım */
static void kullanim(const char *prog) {
    printf("CtrLang %s\n", CTR_SURUM);
    printf("Kullanim: %s [secenekler] <dosya.ctr>\n\n", prog);
    printf("Secenekler:\n");
    printf("  -o <dosya>    Cikti C dosyasi\n");
    printf("  --derle       C'ye cevir ve derle\n");
    printf("  --calistir    Derle ve calistir\n");
    printf("  --ast         AST'yi yazdir\n");
    printf("  --token       Token'lari yazdir\n");
    printf("  --platform    Platform bilgisi\n");
    printf("  --surum       Surum\n");
    printf("  --yardim      Bu mesaj\n");
    printf("  update        CtrLang'i guncelle\n");
}

/* Token modu */
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

/* Derleme */
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
    printf("Derleniyor...\n");
    return system(komut);
}

/* CtrLang guncelleme */
static void ctr_guncelle(void) {
    char dizin[512];
    const char *home = getenv("HOME");
    if (!home) home = ".";

    snprintf(dizin, sizeof(dizin), "%s/CtrLang", home);

    /* Dizin var mi? */
    char kontrol[600];
    snprintf(kontrol, sizeof(kontrol), "%s/.git", dizin);

#ifndef _WIN32
    if (access(kontrol, F_OK) != 0) {
        fprintf(stderr, "\n[HATA] CtrLang dizini bulunamadi: %s\n", dizin);
        fprintf(stderr, "Cozum: yeniden kurun:\n");
        fprintf(stderr, "  git clone https://github.com/darking053official/CtrLang.git\n\n");
        exit(1);
    }
#endif

    printf("\n=== CtrLang Guncelleme ===\n\n");
    printf("Dizin: %s\n\n", dizin);

    /* 1. git pull */
    printf("-> git pull...\n");
    char komut[2048];
    snprintf(komut, sizeof(komut), "cd \"%s\" && git pull", dizin);
    if (system(komut) != 0) {
        fprintf(stderr, "\n[HATA] git pull basarisiz\n\n");
        exit(1);
    }

    /* 2. Yeniden derle ve kur */
    printf("\n-> Yeniden derleniyor...\n");
#ifdef _WIN32
    snprintf(komut, sizeof(komut), "cd \"%s\" && kur.bat", dizin);
#else
    snprintf(komut, sizeof(komut), "cd \"%s\" && chmod +x kur.sh && ./kur.sh", dizin);
#endif

    if (system(komut) != 0) {
        fprintf(stderr, "\n[HATA] Derleme basarisiz\n\n");
        exit(1);
    }

    printf("\n=== Guncelleme tamam! ===\n\n");
    printf("Test:\n");
    printf("  ctr --platform\n");
    printf("  ctr --surum\n\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        kullanim(argv[0]);
        return 1;
    }

    const char *girdi = NULL;
    const char *cikti = NULL;
    int ast_modu = 0;
    int token_modu_flag = 0;
    int derle_flag = 0;
    int calistir_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--yardim") == 0) {
            kullanim(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--surum") == 0) {
            printf("CtrLang %s\n", CTR_SURUM);
            return 0;
        }
        if (strcmp(argv[i], "--platform") == 0) {
            ctr_platform_yazdir();
            return 0;
        }
        if (strcmp(argv[i], "update") == 0 ||
            strcmp(argv[i], "guncelle") == 0) {
            ctr_guncelle();
            return 0;
        }
        if (strcmp(argv[i], "--ast") == 0) { ast_modu = 1; continue; }
        if (strcmp(argv[i], "--token") == 0) { token_modu_flag = 1; continue; }
        if (strcmp(argv[i], "--derle") == 0) { derle_flag = 1; continue; }
        if (strcmp(argv[i], "--calistir") == 0) {
            derle_flag = 1;
            calistir_flag = 1;
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

    /* Dosya oku */
    char *kaynak = dosya_oku(girdi);
    if (!kaynak) {
        ctr_hata_basit(HATA_DOSYA, "Dosya acilamadi");
    }

    /* Token modu */
    if (token_modu_flag) {
        token_modu(kaynak);
        free(kaynak);
        return 0;
    }

    /* Bellek */
    Arena *arena = arena_olustur(0);

    /* Parser */
    Parser p;
    parser_baslat(&p, kaynak, arena);
    ASTDugum *kok = parser_calistir(&p);

    /* AST modu */
    if (ast_modu) {
        printf("=== AST ===\n");
        ast_yazdir(kok, 0);
        free(kaynak);
        arena_yok_et(arena);
        return 0;
    }

    /* Cikti dosyasi */
    char varsayilan_c[512];
    if (!cikti) {
        snprintf(varsayilan_c, sizeof(varsayilan_c), "%s.c", girdi);
        cikti = varsayilan_c;
    }

    FILE *f = fopen(cikti, "w");
    if (!f) {
        ctr_hata_basit(HATA_DOSYA, "Cikti dosyasi acilamadi");
    }

    /* C kodu uret */
    uret_baslat(f);
    uret_program(f, kok);
    uret_bitir(f);
    fclose(f);

    printf("Uretildi: %s\n", cikti);

    /* Derleme */
    if (derle_flag) {
        char cikti_bin[512];
        strncpy(cikti_bin, girdi, sizeof(cikti_bin) - 1);
        cikti_bin[sizeof(cikti_bin) - 1] = '\0';
        char *nokta = strrchr(cikti_bin, '.');
        if (nokta) *nokta = '\0';

        if (gcc_derle(cikti, cikti_bin) != 0) {
            ctr_hata_basit(HATA_URET, "Derleme basarisiz");
        }

        printf("Derlendi: %s\n", cikti_bin);

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
