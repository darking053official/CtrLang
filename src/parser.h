#ifndef CTR_PARSER_H
#define CTR_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "bellek.h"

typedef struct {
    Lexer lexer;
    Token aktif;
    Token onceki;
    Arena *arena;
    int hata_var;
} Parser;

void parser_baslat(Parser *p, const char *kaynak, Arena *arena);
ASTDugum* parser_calistir(Parser *p);

#endif