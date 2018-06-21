#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "commonLib.h"

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

// interpretarea mesaj stdin
// pe ce canal sa se trimita
// daca sa se trimita
int stdinMsg(char *buffer_copy, int *send_to_tcp, int *send_to_udp,
    int is_logged_in, char *last_login_id) {
    const char s[2] = " ";
    char *token;
    char comanda[32];
    int len;

    // primul argument din comanda data
    token = strtok(buffer_copy, s);
    strcpy(comanda, token);
    len = strlen(comanda);
    if (len > 1 && comanda[len - 1] == '\n') {
        comanda[len - 1] = 0;
    }

    if (strcmp(comanda, "login") == 0) {
        // salvam ultimul id introdus, pentru folosirea unlock
        if (token != NULL) {
            token = strtok(NULL, s);
            if (token != NULL) {
                strcpy(last_login_id, token);
            }
        }

        // cand clientul e logat, nu se mai trimite comanda
        if (is_logged_in == 1) {
            memset(buffer_copy, 0, BUFLEN);
            eroareSesiuneDeschisa(buffer_copy, CLIENT_SELF_MSG);
            *send_to_tcp = 0;
            *send_to_udp = 0;
            return -1;
        }
        // cand clientul nu este logat, celelalte comenzi nu se mai trimit la server
    } else if (strcmp(comanda, "logout") == 0 && is_logged_in == 0) {
        memset(buffer_copy, 0, BUFLEN);
        eroareClientNelogat(buffer_copy, CLIENT_SELF_MSG);
        *send_to_tcp = 0;
        *send_to_udp = 0;
        return -1;
    } else if (strcmp(comanda, "listsold") == 0 && is_logged_in == 0) {
        memset(buffer_copy, 0, BUFLEN);
        eroareClientNelogat(buffer_copy, CLIENT_SELF_MSG);
        *send_to_tcp = 0;
        *send_to_udp = 0;
        return -1;
    } else if (strcmp(comanda, "transfer") == 0 && is_logged_in == 0) {
        memset(buffer_copy, 0, BUFLEN);
        eroareClientNelogat(buffer_copy, CLIENT_SELF_MSG);
        *send_to_tcp = 0;
        *send_to_udp = 0;
        return -1;
    } else if (strcmp(comanda, "unlock") == 0) {
        // daca se scrie unlock, atunci se trimite pe udp, impreuna cu
        // id-ul anterior
        memset(buffer_copy, 0, BUFLEN);
        sprintf(buffer_copy, "unlock %s", last_login_id);
        *send_to_tcp = 0;
        *send_to_udp = 1;
        return 0;
    }

    *send_to_tcp = 1;
    *send_to_udp = 0;
    return 0;
}


int main(int argc, char *argv[]) {
    // variabile conexiune client-server
    struct sockaddr_in serv_addr;
    int udp_sock, tcp_sock, portno;
    char buffer[BUFLEN];
    char buffer_copy[BUFLEN];
    fd_set read_fds;
    int fdmax;
    int msg_size;
    socklen_t sockaddr_size = sizeof(struct sockaddr);
    int send_to_udp, send_to_tcp;
    // variabile program banking
    char last_login_id[32];
    int is_logged_in = 0;
    int is_transfer_question = 0;
    int is_parola_trimitere = 0;
    FILE *log;

    if (argc < 3) {
       exit(0);
    }

    memset(buffer, 0, BUFLEN);
    sprintf(buffer, "client_%d.log", getpid());
    log = fopen(buffer, "w");

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (udp_sock < 0 || tcp_sock < 0) {
        perror("Creare socketi");
        exit(0);
    }

    portno = atoi(argv[2]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    inet_aton(argv[1], &serv_addr.sin_addr);

    if (connect(tcp_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("tcp connect");
        exit(0);
    }

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN, &read_fds);
        FD_SET(tcp_sock, &read_fds);
        FD_SET(udp_sock, &read_fds);

        fdmax = max(STDIN, tcp_sock);
        fdmax = max(udp_sock, fdmax);

        send_to_tcp = 0;
        send_to_udp = 0;


        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Error select");
        }

        // mesaj venit tcp
        if (FD_ISSET(tcp_sock, &read_fds)) {
            memset(buffer, 0, BUFLEN);
            msg_size = recv(tcp_sock, buffer, BUFLEN, 0);
            if (msg_size < 0) {
                perror("tcp recv");
                exit(0);
            }
            if (isQuitMsg(buffer) == 1 || msg_size == 0) {
                break;
            }
            printf("%s\n", buffer);
            fprintf(log, "%s\n", buffer);
            // se observa daca este un mesaj care restrictioneaza
            // anumite proprietati
            if (isWelcomeMsg(buffer) == 1) {
                is_logged_in = 1;
            }
            if (isDeconectatMsg(buffer) == 1) {
                is_logged_in = 0;
            }
            if (isTransferMsg(buffer) == 1) {
                is_transfer_question = 1;
            }
        }

        //mesaj venit stdin
        if (FD_ISSET(STDIN, &read_fds)) {
            memset(buffer, 0, BUFLEN);
            if (read(STDIN, buffer, BUFLEN) < 0) {
                perror("stdin read");
                exit(0);
            }
            //printf("recieved stdin: %s\n", buffer);
            fprintf(log, "%s", buffer);

            strcpy(buffer_copy, buffer);
            // mesajul se trimite in mod normal daca
            // nu este momentul cand este cerut un mesaj care nu e de tip
            // comanda. de exemplu, la unlock, se cere un string oarecare
            if (is_transfer_question == 0 && is_parola_trimitere == 0) {
                if (stdinMsg(buffer_copy, &send_to_tcp, &send_to_udp, is_logged_in,
                    last_login_id) < 0) {
                    printf("%s\n", buffer_copy);
                    fprintf(log, "%s\n", buffer_copy);
                    continue;
                }
            }

            // daca se trimitea parola de la unlock, se va lipi si
            // ultimul id, in mesaj
            if (is_parola_trimitere == 1) {
                memset(buffer_copy, 0, BUFLEN);
                sprintf(buffer_copy, "%s ", last_login_id);
                strcat(buffer_copy, buffer);
            }

            // se trimite pe tcp daca s-a decis anterior sa se trimita pe tcp,
            // de catre functia stdinMSG, sau daca este interogat clientul
            // sa raspunda cu y/n
            if (send_to_tcp || is_transfer_question == 1) {
                is_transfer_question = 0;
                msg_size = send(tcp_sock, buffer, strlen(buffer), 0);
                if (msg_size < 0) {
                    perror("tcp send");
                }
            }
            // se trimite pe udp daca s-a decis anterior sa se trimita.
            // sau daca se cere parola
            if (send_to_udp || is_parola_trimitere == 1) {
                is_parola_trimitere = 0;
                if (sendto(udp_sock, buffer_copy, strlen(buffer_copy) + 1, 0,
                    (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
                    perror("udp send");
                    break;
                }
            }
            if (isQuitMsg(buffer) == 1) {
                break;
            }
        }

        // mesaj venit udp
        if (FD_ISSET(udp_sock, &read_fds)) {
            memset(buffer, 0, BUFLEN);
            if (recvfrom(udp_sock, buffer, BUFLEN, 0, (struct sockaddr*)&serv_addr,
                &sockaddr_size) < 0) {
                perror("udp recv");
                continue;
            }
            if (isTrimiteParola(buffer) == 1) {
                is_parola_trimitere = 1;
            }
            printf("%s\n", buffer);
            fprintf(log, "%s\n", buffer);
        }

    }

    FD_CLR(udp_sock, &read_fds);
    FD_CLR(tcp_sock, &read_fds);
    FD_CLR(STDIN, &read_fds);
    close(udp_sock);
    close(tcp_sock);
    fclose(log);
    
    return 0;
}