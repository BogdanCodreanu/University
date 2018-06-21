#ifndef UTILITY_DATA
#define UTILITY_DATA
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	char nume[12];
	char prenume[12];
	short numarCard[6];
	short pin[4];
	char parola[8];
	double sold;
} ContBancar;

typedef struct {
	ContBancar *conturi;
	int nrConturi;
} Banking;


Banking GenerateBankingFromFile(char *filename);
void DestructBanking(Banking *banking);

void PrintCont(ContBancar *cont);
void InitContFromFile(ContBancar *cont, FILE *fp);



#endif