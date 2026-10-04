#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "hata.h"
#include "bellek.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "uret.h"

static char* dosya_oku(const char *yol) {
    FILE *f = fopen(yol, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long boyut = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *icerik = malloc(boyut + 1);
    if (!icerik) { fclose(f); return NULL; }
    
    fread(icerik, 1, boyut, f);
    icerik[boyut] = '\0';
    
    fclose(f);
    return icerik;
}

static void kullanim(const char *prog) {
    printf("CtrLang %s\n", CTR_SURUM);
    printf("Kullanım: %s [seçenekler] <dosya.ctr>\n\n", prog);
    printf("Seçenekler:\n");
    printf("  -o <dosya>    Çıktı C dosyası (varsayılan: <girdi>.c)\n");
    printf("  --derle       C'ye çevir ve gcc ile derle\n");
    printf("  --calistir    Derle ve çalıştır\n");
    printf("  --ast         AST'yi yazdır\n");
    printf("  --token       Token'ları yazdır\n");
    printf("  --platform    Platform bilgisi\n");
    printf("  --surum       Sürüm\n");
    printf("  --yardim      Bu mesaj\n");
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
    char komut[1024];
    snprintf(komut, sizeof(komut),
             "gcc -std=c11 -O2 -I kutuphane -o %s %s kutuphane/*.c -lpthread -lm",
             cikti, c_dosya);
    printf("Derleniyor: %s\n", komut);
    return system(komut);
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
        if (strcmp(argv[i], "--ast") == 0) { ast_modu = 1; continue; }
        if (strcmp(argv[i], "--token") == 0) { token_modu_flag = 1; continue; }
        if (strcmp(argv[i], "--derle") == 0) { derle_flag = 1; continue; }
        if (strcmp(argv[i], "--calistir") == 0) { 
            derle_flag = 1; calistir_flag = 1; continue; 
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
    
    /* Dosyayı oku */
    char *kaynak = dosya_oku(girdi);
    if (!kaynak) {
        ctr_hata_basit(HATA_DOSYA, "Dosya açılamadı");
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
    
    /* Çıktı dosyası */
    char varsayilan_c[512];
    if (!cikti) {
        snprintf(varsayilan_c, sizeof(varsayilan_c), "%s.c", girdi);
        cikti = varsayilan_c;
    }
    
    FILE *f = fopen(cikti, "w");
    if (!f) {
        ctr_hata_basit(HATA_DOSYA, "Çıktı dosyası açılamadı");
    }
    
    /* C kodu üret */
    uret_baslat(f);
    uret_program(f, kok);
    uret_bitir(f);
    
    fclose(f);
    
    printf("Üretildi: %s\n", cikti);
    
    /* Derleme */
    if (derle_flag) {
        char cikti_bin[512];
        strncpy(cikti_bin, girdi, sizeof(cikti_bin));
        char *nokta = strrchr(cikti_bin, '.');
        if (nokta) *nokta = '\0';
        
        if (gcc_derle(cikti, cikti_bin) != 0) {
            ctr_hata_basit(HATA_URET, "Derleme başarısız");
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