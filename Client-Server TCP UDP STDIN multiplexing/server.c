#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "commonLib.h"

#define MAX_CLIENTS 5

struct Cont {
    char nume[13];
    char prenume[13];
    char numar_card[7];
    char pin[5];
    char parola_secreta[9];
    double sold;
};

struct TransferInProgress {
    int cont_index_to;
    double sold_transfered;
};


int isCont(struct Cont cont, char *searched_numar) {
    if (strcmp(searched_numar, cont.numar_card) == 0)
        return 1;
    return 0;
}

int isContLiber(int index_cont_dorit, int *tcp_logged_into) {
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (tcp_logged_into[i] == index_cont_dorit)
            return 0;
    }
    return 1;
}

// functie care verifica comanda venita pe tcp si modifica valoarea
// conturilor bancare. returneaza si un raspuns catre client, in response_buffer
int mesajTCP(char *buffer, char *response_buffer, int *tcp_logged_into,
    struct Cont *conts, int nr_conturi, int *conts_blocked, int tcp_index,
    struct TransferInProgress *transfers) {

    const char s[2] = " ";
    char *token;
    int len, i, cont_gasit;
    int n_args = 0;
    char args[3][40];
    // erorile de pin pentru fiecare client tcp
    static int pin_error[MAX_CLIENTS];
    static int cont_gasit_anterior[MAX_CLIENTS];
    static int need_to_initialize_vars = 0;

    printf("[TCP] %s", buffer);

    if (need_to_initialize_vars == 0) {
        need_to_initialize_vars = 1;
        for (i = 0; i < MAX_CLIENTS; i++) {
            pin_error[i] = 0;
            cont_gasit_anterior[i] = -2;
        }
    }

    if (isQuitMsg(buffer)) {
        printf("has quit.\n");
        return -1;
    }
    memset(response_buffer, 0, BUFLEN);

    // extragerea argumentelor date in comanda
    token = strtok(buffer, s);
    while (token != NULL) {
        if (n_args < 3) {
            strcpy(args[n_args], token);
            
            len = strlen(args[n_args]);
            if (len > 1 && args[n_args][len - 1] == '\n') {
                args[n_args][len - 1] = 0;
            }
            printf("-%s-", args[n_args]);
        }
        printf("\n");
        n_args++;
        token = strtok(NULL, s);
    }


    // daca este momentul in care se asteapta un raspuns de y/n pentru transfer bancar
    if (transfers[tcp_index].cont_index_to != -1) {
        cont_gasit = transfers[tcp_index].cont_index_to;
        if (n_args == 1 && strcmp(args[0], "y") == 0) {
            conts[cont_gasit].sold += transfers[tcp_index].sold_transfered;
            conts[tcp_logged_into[tcp_index]].sold -= transfers[tcp_index].sold_transfered;
            transfers[tcp_index].cont_index_to = -1;
            mesajTransferSucces(response_buffer, IBANK_MSG);
            return 0;
        }
        transfers[tcp_index].cont_index_to = -1;
        eroareOperatieAnulata(response_buffer, IBANK_MSG);
        return 0;
    }

    if (strcmp(args[0], "login") == 0) {
        // comanda login

        // daca este apelata gresit
        if (n_args != 3) {
            eroareApelFunctie(response_buffer, IBANK_MSG, args[0]);
            return 0;
        }
        cont_gasit = -1;
        for (i = 0; i < nr_conturi; i++) {
            if (isCont(conts[i], args[1])) {
                cont_gasit = i;
                break;
            }
        }
        // daca nu exista contul
        if (cont_gasit == -1) {
            eroareNumarCardInexistent(response_buffer, IBANK_MSG);
            return 0;
        }

        // daca contul introdus este un cont diferit fata de cel precedent
        // resetam incercarile-eroare de pin
        if (cont_gasit_anterior[tcp_index] != cont_gasit) {
            cont_gasit_anterior[tcp_index] = cont_gasit;
            pin_error[tcp_index] = 0;
        }

        // daca e contul blocat
        if (conts_blocked[cont_gasit] == 1) {
            eroareCardBlocat(response_buffer, IBANK_MSG);
            return 0;
        }

        // daca e pinul gresit
        if (strcmp(conts[cont_gasit].pin, args[2]) != 0) {
            pin_error[tcp_index]++;
            if (pin_error[tcp_index] >= 3) {
                conts_blocked[cont_gasit] = 1;
                eroareCardBlocat(response_buffer, IBANK_MSG);
                return 0;
            } else {
                eroarePinGresit(response_buffer, IBANK_MSG);
            }
            return 0;
        }

        // daca nu este liber aceset cont (alt client e logat pe el)
        if (isContLiber(cont_gasit, tcp_logged_into) == 0) {
            eroareSesiuneDeschisa(response_buffer, IBANK_MSG);
            return 0;
        }

        mesajWelcome(response_buffer, IBANK_MSG, conts[cont_gasit].nume,
            conts[cont_gasit].prenume);
        pin_error[tcp_index] = 0;
        tcp_logged_into[tcp_index] = cont_gasit;
        return 0;
    } else if (strcmp(args[0], "logout") == 0) {
        // comanda logout

        mesajDeconectare(response_buffer, IBANK_MSG);
        tcp_logged_into[tcp_index] = -1;
        return 0;
    } else if (strcmp(args[0], "listsold") == 0) {
        // comanda listsold

        if (tcp_logged_into[tcp_index] == -1) {
            eroareClientNelogat(response_buffer, IBANK_MSG);
            return 0;
        }
        mesajSold(response_buffer, IBANK_MSG, conts[tcp_logged_into[tcp_index]].sold);
        return 0;
    } else if (strcmp(args[0], "transfer") == 0) {
        // comanda transfer
        double sold_cerut;

        if (n_args != 3) {
            eroareApelFunctie(response_buffer, IBANK_MSG, args[0]);
            return 0;
        }

        if (tcp_logged_into[tcp_index] == -1) {
            eroareClientNelogat(response_buffer, IBANK_MSG);
            return 0;
        }

        // contul catre care voi transfera
        cont_gasit = -1;
        for (i = 0; i < nr_conturi; i++) {
            if (isCont(conts[i], args[1])) {
                cont_gasit = i;
                break;
            }
        }
        // daca nu e cont gasit
        if (cont_gasit == -1) {
            eroareNumarCardInexistent(response_buffer, IBANK_MSG);
            return 0;
        }
        // daca soldu lcerut este prea mare
        sscanf(args[2], "%lf", &sold_cerut);
        if (sold_cerut > conts[tcp_logged_into[tcp_index]].sold) {
            eroareFonduriInsuficiente(response_buffer, IBANK_MSG);
            return 0;
        }

        // trimit mesaj de "esti sigur ca vrei sa trimiti banii?"
        // si marchez tranzactia ce urmeaza sa fie facuta
        transfers[tcp_index].cont_index_to = cont_gasit;
        transfers[tcp_index].sold_transfered = sold_cerut;
        mesajAgreeTransfer(response_buffer, IBANK_MSG, args[2], conts[cont_gasit].nume,
            conts[cont_gasit].prenume);
        return 0;
    }
    eroareApelFunctie(response_buffer, IBANK_MSG, args[0]);
    return 0;
}

int mesajUDP(char *buffer, char *response_buffer,
    struct Cont *conts, int nr_conturi, int *conts_blocked) {

    const char s[2] = " ";
    char *token;
    int len, i, cont_gasit;
    int n_args = 0;
    char args[3][40];

    memset(response_buffer, 0, BUFLEN);

    // extragerea argumentelor date in comanda
    token = strtok(buffer, s);
    while (token != NULL) {
        if (n_args < 3) {
            strcpy(args[n_args], token);
            
            len = strlen(args[n_args]);
            if (len > 1 && args[n_args][len - 1] == '\n') {
                args[n_args][len - 1] = 0;
            }
            printf("-%s-", args[n_args]);
        }
        printf("\n");
        n_args++;
        token = strtok(NULL, s);
    }

    if (strcmp(args[0], "unlock") == 0) {
        // momentul in care se primeste comanda unlock
        cont_gasit = -1;
        for (i = 0; i < nr_conturi; i++) {
            if (isCont(conts[i], args[1])) {
                cont_gasit = i;
                break;
            }
        }

        if (cont_gasit == -1) {
            eroareNumarCardInexistent(response_buffer, UNLOCK_MSG);
            return 0;
        }

        if (conts_blocked[cont_gasit] == 0) {
            eroareOperatieEsuata(response_buffer, UNLOCK_MSG);
            return 0;
        }
        mesajTrimiteParola(response_buffer, UNLOCK_MSG);
        return 0;
    } else {
        // a venit mesajul cu parola
        cont_gasit = -1;
        for (i = 0; i < nr_conturi; i++) {
            if (isCont(conts[i], args[0])) {
                cont_gasit = i;
                break;
            }
        }

        if (cont_gasit == -1) {
            eroareNumarCardInexistent(response_buffer, UNLOCK_MSG);
            return 0;
        }

        if (conts_blocked[cont_gasit] == 0) {
            eroareOperatieEsuata(response_buffer, UNLOCK_MSG);
            return 0;
        }

        if (strcmp(conts[cont_gasit].parola_secreta, args[1]) != 0) {
            eroareDeblocareEsuata(response_buffer, UNLOCK_MSG);
            return 0;
        }

        mesajClientDeblocat(response_buffer, UNLOCK_MSG);
        conts_blocked[cont_gasit] = 0;
        return 0;
    }

    return 0;
}

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, cli_addr;
    int udp_sock, tcp_sock, portno, new_tcp_sock;
    char buffer[BUFLEN];
    char response_buffer[BUFLEN];
    fd_set read_fds;
    int fdmax, i;
    int msg_size;
    int tcp_socks_known[MAX_CLIENTS];
    socklen_t sockaddr_size = sizeof(struct sockaddr);
    int sock_opt = 1;
    // variabile banking
    struct Cont *conts;
    int nr_conturi;
    FILE *conts_file;
    // socketul respectiv este logat pe contul cu voloarea asta
    int tcp_logged_into[MAX_CLIENTS];
    struct TransferInProgress transfers[MAX_CLIENTS];
    int *conts_blocked;

    if (argc < 3) {
       exit(0);
    }

    // citire din fisier
    conts_file = fopen(argv[2], "r");
    fscanf(conts_file, "%d", &nr_conturi);
    conts = malloc(nr_conturi * sizeof(struct Cont));
    conts_blocked = malloc(nr_conturi * sizeof(int));
    for (i = 0; i < nr_conturi; i++) {
        fscanf(conts_file, "%s", conts[i].nume);
        fscanf(conts_file, "%s", conts[i].prenume);
        fscanf(conts_file, "%s", conts[i].numar_card);
        fscanf(conts_file, "%s", conts[i].pin);
        fscanf(conts_file, "%s", conts[i].parola_secreta);
        fscanf(conts_file, "%lf", &conts[i].sold);
        conts_blocked[i] = 0;
    }
    fclose(conts_file);

    for (i = 0; i < MAX_CLIENTS; i++) {
        tcp_socks_known[i] = 0;
        tcp_logged_into[i] = -1;
        transfers[i].cont_index_to = -1;
    }

    // initializare socketi
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (udp_sock < 0 || tcp_sock < 0) {
        perror("Creare socketi");
        exit(0);
    }

    portno = atoi(argv[1]);

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));


    if (bind(tcp_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0) {
        perror("Bind tcp");
        exit(0);
    }

    if (bind(udp_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0) {
        perror("Bind udp");
        exit(0);
    }

    if (listen(tcp_sock, MAX_CLIENTS) < 0) {
        perror("Listen initial");
        exit(0);
    }

    while (1) {
        FD_ZERO(&read_fds);
        // adaugam toti socketii de pe care sa ascultam
        FD_SET(tcp_sock, &read_fds);
        FD_SET(STDIN, &read_fds);
        FD_SET(udp_sock, &read_fds);

        fdmax = max(tcp_sock, STDIN);
        fdmax = max(udp_sock, fdmax);
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (tcp_socks_known[i] > 0) {
                // + socketii clientilor existenti
                FD_SET(tcp_socks_known[i], &read_fds);
                if (tcp_socks_known[i] > fdmax) {
                    fdmax = tcp_socks_known[i];
                }
            }
        }

        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Error select");
        }

        // daca vine ceva pe socketul tcp poate fi doar o incercare de conectare
        if (FD_ISSET(tcp_sock, &read_fds)) {
            new_tcp_sock = accept(tcp_sock, (struct sockaddr*)&cli_addr,
                &sockaddr_size);
            if (new_tcp_sock < 0) {
                perror("tcp accept");
                continue;
            }
            printf("conexiune noua: %d\n", new_tcp_sock);
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (tcp_socks_known[i] == 0) {
                    tcp_socks_known[i] = new_tcp_sock;
                    if (new_tcp_sock > fdmax) {
                        fdmax = new_tcp_sock;
                    }
                    break;
                }
                if (i == MAX_CLIENTS - 1) {
                    // refuzarea unui client nou
                    printf("Nr clienti maxim atinsi\n");
                    close(new_tcp_sock);
                    FD_CLR(new_tcp_sock, &read_fds);
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            // daca vine ceva pe socketul clientilor
            if (tcp_socks_known[i] != 0 &&
                FD_ISSET(tcp_socks_known[i], &read_fds)) {

                memset(buffer, 0, BUFLEN);
                msg_size = recv(tcp_socks_known[i], buffer, BUFLEN, 0);

                if (msg_size <= 0) {
                    if (msg_size == 0) {
                        printf("tcp %d deconectat\n", tcp_socks_known[i]);
                    } else {
                        perror("tcp recv msg");
                    }
                    close(tcp_socks_known[i]);
                    FD_CLR(tcp_socks_known[i], &read_fds);
                    tcp_socks_known[i] = 0;
                    tcp_logged_into[i] = -1;
                    transfers[i].cont_index_to = -1;
                } else {
                    // se calculeaza ce inseamna comanda primita
                    if (mesajTCP(buffer, response_buffer, tcp_logged_into,
                        conts, nr_conturi, conts_blocked, i, transfers) < 0) {
                        // daca e mai mica decat 0, inseamna ca 
                        // clientul s-a deconectat
                        close(tcp_socks_known[i]);
                        FD_CLR(tcp_socks_known[i], &read_fds);
                        tcp_socks_known[i] = 0;
                        tcp_logged_into[i] = -1;
                        transfers[i].cont_index_to = -1;
                    } else {
                        // raspund cu un mesaj clientului
                        msg_size = send(tcp_socks_known[i], response_buffer,
                            strlen(response_buffer), 0);
                        if (msg_size < 0) {
                            perror("tcp send");
                        }
                    }
                }
            }
        }

        // mesaj venit udp
        if (FD_ISSET(udp_sock, &read_fds)) {
            memset(buffer, 0, BUFLEN);
            if (recvfrom(udp_sock, buffer, BUFLEN, 0, (struct sockaddr*)&cli_addr,
                &sockaddr_size) < 0) {
                perror("udp recv");
                continue;
            }

            printf("[UDP] %s\n\n", buffer);
            // calculez mesajul udp
            mesajUDP(buffer, response_buffer, conts, nr_conturi, conts_blocked);
            // si trimit raspunsul
            if (sendto(udp_sock, response_buffer, strlen(response_buffer) + 1, 0,
                (struct sockaddr *)&cli_addr, sizeof(struct sockaddr)) < 0) {
                perror("udp send");
                break;
            }
        }

        // mesaj venit stdin
        if (FD_ISSET(STDIN, &read_fds)) {
            memset(buffer, 0, BUFLEN);
            if (read(STDIN, buffer, BUFLEN) < 0) {
                perror("stdin read");
                continue;
            }
            printf("[STDIN] %s", buffer);
            // daca primesc quit de la stdin, trimit mesajul de inchidere
            // catre toti clientii
            if (strcmp(buffer, "quit\n") == 0) {
                printf("broadcasting quit msg.\n");
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (tcp_socks_known[i] > 0) {
                        msg_size = send(tcp_socks_known[i], buffer, strlen(buffer), 0);
                        if (msg_size < 0) {
                            perror("tcp send");
                        }
                        close(tcp_socks_known[i]);
                        FD_CLR(tcp_socks_known[i], &read_fds);
                    }
                }
                break;
            }
        }
    }

    FD_CLR(udp_sock, &read_fds);
    FD_CLR(tcp_sock, &read_fds);
    FD_CLR(STDIN, &read_fds);
    close(udp_sock);
    close(tcp_sock);
    free(conts);
    free(conts_blocked);
    return 0;
}