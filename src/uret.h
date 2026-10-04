#ifndef CTR_URET_H
#define CTR_URET_H

#include "ast.h"
#include <stdio.h>

/* Global parametreler (main.c'den) */
extern int g_port;
extern char g_port_env[128];
extern char g_host[128];
extern int g_debug;
extern int g_verbose;
extern int g_quiet;

/* AST → C kodu */
void uret_baslat(FILE *cikis);
void uret_bitir(FILE *cikis);
void uret_dugum(FILE *cikis, ASTDugum *d, int girinti);
void uret_program(FILE *cikis, ASTDugum *kok);

#endif
