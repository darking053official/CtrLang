#ifndef CTR_VERITABANI_H
#define CTR_VERITABANI_H

#include "ctr_calisma.h"

void ctr_veritabani_baglan(const char *dosya);
void ctr_veritabani_kapat(void);
void ctr_veritabani_calistir(const char *sql);
ctr_liste* ctr_veritabani_sorgu(const char *sql);

#endif
