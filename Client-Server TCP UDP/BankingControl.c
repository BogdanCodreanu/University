#include "BankingControl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void PrintCont(ContBancar *cont) {
	int i;
	printf("%s %s ", cont->nume, cont->prenume);

	for (i = 0; i < 6; i++) {
		printf("%hi", cont->numarCard[i]);
	}
	printf(" ");
	for (i = 0; i < 4; i++) {
		printf("%hi", cont->pin[i]);
	}
	printf(" %s %lf\n", cont->parola, cont->sold);
}


void InitContFromFile(ContBancar *cont, FILE *fp) {
	char nrCard[6];
	char pin[4];
	int i;
	fscanf(fp, "%s%s%s%s%s%lf", cont->nume, cont->prenume, nrCard, pin, cont->parola, &cont->sold);

	for (i = 0; i < 6; i++) {
		cont->numarCard[i] = (short)nrCard[i] - 48;
	}

	for (i = 0; i < 4; i++) {
		cont->pin[i] = (short)pin[i] - 48;
	}
}

Banking GenerateBankingFromFile(char *filename) {
	Banking banking;
	FILE *fp;
	int i;

	fp = fopen(filename, "r");
	fscanf(fp, "%d", &banking.nrConturi);
	banking.conturi = (ContBancar*)malloc(banking.nrConturi * sizeof(ContBancar));

	for (i = 0; i < banking.nrConturi; i++) {
		InitContFromFile(&banking.conturi[i], fp);
	}
	fclose(fp);
	return banking;
}

void DestructBanking(Banking *banking) {
	free(banking->conturi);
}
