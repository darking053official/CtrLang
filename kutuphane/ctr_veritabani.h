#ifndef CTR_VERITABANI_H
#define CTR_VERITABANI_H

void ctr_veritabani_baglan(const char *dosya);
void ctr_veritabani_kapat(void);
void ctr_veritabani_calistir(const char *sql);
int  ctr_veritabani_sorgu(const char *sql);
int  ctr_veritabani_satir_sayi(void);
const char* ctr_veritabani_alan(int satir, const char *alan);
const char* ctr_veritabani_alan_indeks(int satir, int sutun);

#endif
