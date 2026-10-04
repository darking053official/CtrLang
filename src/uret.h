#ifndef CTR_URET_H
#define CTR_URET_H

#include "ast.h"
#include <stdio.h>

/* AST → C kodu */
void uret_baslat(FILE *cikis);
void uret_bitir(FILE *cikis);
void uret_dugum(FILE *cikis, ASTDugum *d, int girinti);
void uret_program(FILE *cikis, ASTDugum *kok);

#endif