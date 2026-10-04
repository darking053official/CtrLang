#include "bellek.h"
#include <stdlib.h>
#include <string.h>

#define BLOK_BOYUT (64 * 1024)

typedef struct Blok {
    struct Blok *sonraki;
    size_t kullanilan;
    size_t boyut;
    char veri[];
} Blok;

struct Arena {
    Blok *ilk;
    Blok *aktif;
};

static Blok* blok_olustur(size_t boyut) {
    Blok *b = malloc(sizeof(Blok) + boyut);
    if (!b) return NULL;
    b->sonraki = NULL;
    b->kullanilan = 0;
    b->boyut = boyut;
    return b;
}

Arena* arena_olustur(size_t baslangic_boyut) {
    Arena *a = malloc(sizeof(Arena));
    if (!a) return NULL;
    
    if (baslangic_boyut == 0) baslangic_boyut = BLOK_BOYUT;
    
    a->ilk = blok_olustur(baslangic_boyut);
    a->aktif = a->ilk;
    return a;
}

void* arena_ayir(Arena *arena, size_t boyut) {
    boyut = (boyut + 7) & ~((size_t)7);
    
    Blok *b = arena->aktif;
    if (b->kullanilan + boyut > b->boyut) {
        size_t yeni_boyut = boyut > BLOK_BOYUT ? boyut : BLOK_BOYUT;
        Blok *yeni = blok_olustur(yeni_boyut);
        if (!yeni) return NULL;
        b->sonraki = yeni;
        arena->aktif = yeni;
        b = yeni;
    }
    
    void *ptr = b->veri + b->kullanilan;
    b->kullanilan += boyut;
    return ptr;
}

char* arena_strdup(Arena *arena, const char *metin) {
    return arena_strndup(arena, metin, strlen(metin));
}

char* arena_strndup(Arena *arena, const char *metin, size_t uzunluk) {
    char *kopya = arena_ayir(arena, uzunluk + 1);
    if (kopya) {
        memcpy(kopya, metin, uzunluk);
        kopya[uzunluk] = '\0';
    }
    return kopya;
}

void arena_yok_et(Arena *arena) {
    Blok *b = arena->ilk;
    while (b) {
        Blok *sonraki = b->sonraki;
        free(b);
        b = sonraki;
    }
    free(arena);
}