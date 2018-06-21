#ifndef COMMON_LIB
#define COMMON_LIB

#include <stdio.h>
#include <string.h>

#define BUFLEN 256
#define STDIN 0


#define IBANK_MSG 0
#define UNLOCK_MSG 1
#define CLIENT_SELF_MSG 2
#define IBANK_PREFIX "IBANK> "
#define UNLOCK_PREFIX "UNLOCK> "

int isQuitMsg(char *buffer) {
    if (strcmp(buffer, "quit\n") == 0)
        return 1;
    return 0;
}

int isWelcomeMsg(char *buffer) {
    const char s[2] = " ";
    char *token;
    char buffer_copy[BUFLEN];
    int i;
    strcpy(buffer_copy, buffer);

    token = strtok(buffer_copy, s);
    if (token == NULL) {
        return 0;
    }
    for (i = 0; i < 1; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "Welcome") == 0) {
        return 1;
    }
    return 0;
}

int isDeconectatMsg(char *buffer) {
    const char s[2] = " ";
    char *token;
    char buffer_copy[BUFLEN];
    int i;
    strcpy(buffer_copy, buffer);

    token = strtok(buffer_copy, s);
    if (token == NULL) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "deconectat") == 0) {
        return 1;
    }
    return 0;
}

int isTransferMsg(char *buffer) {
    const char s[2] = " ";
    char *token;
    char buffer_copy[BUFLEN];
    int i;
    strcpy(buffer_copy, buffer);

    token = strtok(buffer_copy, s);
    if (token == NULL) {
        return 0;
    }
    for (i = 0; i < 1; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "Transfer") != 0) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "catre") == 0) {
        return 1;
    }

    return 0;
}

int isTrimiteParola(char *buffer) {
    const char s[2] = " ";
    char *token;
    char buffer_copy[BUFLEN];
    int i;
    strcpy(buffer_copy, buffer);

    token = strtok(buffer_copy, s);
    if (token == NULL) {
        return 0;
    }
    for (i = 0; i < 1; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "Trimite") != 0) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        token = strtok(NULL, s);
        if (token == NULL) {
            return 0;
        }
    }
    if (strcmp(token, "secreta") == 0) {
        return 1;
    }

    return 0;
}


// coduri eroare
void eroareClientNelogat(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-1 : Clientul nu este autentificat");
}

void eroareSesiuneDeschisa(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-2 : Sesiune deja deschisa");
}

void eroarePinGresit(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-3 : Pin gresit");
}

void eroareNumarCardInexistent(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-4 : Numar card inexistent");
}

void eroareCardBlocat(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-5 : Card blocat");
}

void eroareOperatieEsuata(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-6 : Operatie esuata");
}

void eroareDeblocareEsuata(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-7 : Deblocare esuata");
}

void eroareFonduriInsuficiente(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-8 : Fonduri insuficiente");
}

void eroareOperatieAnulata(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-9 : Operatie anulata");
}

void eroareApelFunctie(char *result, int from, char *nume_functie) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "-10 : Eroare la apel ");
    strcat(result, nume_functie);
}

void mesajWelcome(char *result, int from, char *nume, char *prenume) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Welcome ");
    strcat(result, nume);
    strcat(result, " ");
    strcat(result, prenume);
}

void mesajDeconectare(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Clientul a fost deconectat");
}

void mesajSold(char *result, int from, double sold) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    char aux[100];
    sprintf(aux, "%.2lf", sold);
    strcat(result, aux);
}

void mesajAgreeTransfer(char *result, int from, char *sold, char *nume, char *prenume) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Transfer ");
    strcat(result, sold);
    strcat(result, " catre ");
    strcat(result, nume);
    strcat(result, " ");
    strcat(result, prenume);
    strcat(result, "? [y/n]");
}

void mesajTransferSucces(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Transfer realizat cu succes");
}

void mesajTrimiteParola(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Trimite parola secreta");
}

void mesajClientDeblocat(char *result, int from) {
    if (from == IBANK_MSG) {
        strcat(result, IBANK_PREFIX);
    } else if (from == UNLOCK_MSG) {
        strcat(result, UNLOCK_PREFIX);
    }
    strcat(result, "Client deblocat");
}


#endif