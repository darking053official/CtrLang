#ifndef CTR_BELLEK_H
#define CTR_BELLEK_H

#include <stddef.h>

typedef struct Arena Arena;

Arena* arena_olustur(size_t baslangic_boyut);
void*  arena_ayir(Arena *arena, size_t boyut);
char*  arena_strdup(Arena *arena, const char *metin);
char*  arena_strndup(Arena *arena, const char *metin, size_t uzunluk);
void   arena_yok_et(Arena *arena);

#endif