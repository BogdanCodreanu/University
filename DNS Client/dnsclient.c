#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "DnsLib.h"

#define DNS_NAME_BUFF_SIZE 256
#define UDP_BUFFER_SIZE 4096
#define QNAME_BUFFER_SIZE 512

#define READ_DATA_CLASSIC 0
#define READ_DATA_ONLY_FIRST 1

int GetNextDns(FILE *dnsFile, char *output);
void ConvertToDnsName(char *hostName, char *dnsName);
int GetQueryFromString(char *str);
void InterpretResponseData(int query, int len, unsigned char *response,
    char *extractedInfo, int msgIndex, unsigned char *wholeMsg);
void LogMessageHexa(FILE *where, char *data, int nrOfBytes);
int ReadDataLabel(char *output, char *from, char *wholeMsg, int flag);
int CheckIfPTR(char *possibleIP, char *queryString);

FILE *dnsLog;
FILE *messageLog;

// cauta un server de dns disponibil. genereaza buffer-ul drept intrebare dns
// si trimite.
void AskDNS(FILE *dnsFile, char *hostName, int query, char *queryString) {
    int i;
    char udpBuffer[UDP_BUFFER_SIZE];
    char dnsServer[DNS_NAME_BUFF_SIZE];
    char savedHostName[DNS_NAME_BUFF_SIZE];
    char extractedInfo[QNAME_BUFFER_SIZE];
    int udpBufferSize;
    struct timeval timeoutVal;

    struct sockaddr_in destination;
    socklen_t sockaddrSize = sizeof(struct sockaddr);
    int socketFD;

    dnsHeaderT dnsHeader, *readDnsHeader;
    char qname[QNAME_BUFFER_SIZE];
    dnsQuestionT qinfo;

    unsigned short nrOfAnswers;
    unsigned int readingOffset;

    static short loggedMessage = 0;

    int errorCodeVal;

    timeoutVal.tv_sec = 0;
    timeoutVal.tv_usec = 100000;

    strcpy(savedHostName, hostName);

    socketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, &timeoutVal, sizeof(timeoutVal)) < 0) {
        perror("Assigning timeout");
        exit(1);
    }

    dnsHeader = CreateDns(getpid());
    ConvertToDnsName(hostName, qname);

    qinfo.qtype = htons(query);
    qinfo.qclass = htons(1);


    do {
        if (GetNextDns(dnsFile, dnsServer) < 0) {
            printf("Limit reached on retrieving next dns (no more dns servers specified)\n");
            exit(1);
        }
        destination.sin_family = AF_INET;
        destination.sin_port = htons(53);
        destination.sin_addr.s_addr = inet_addr(dnsServer);

        // adaugam header-ul, qname-ul si qinfo-ul in buffer;
        memset(udpBuffer, 0, UDP_BUFFER_SIZE);
        udpBufferSize = 0;

        memcpy(udpBuffer, &dnsHeader, sizeof(dnsHeaderT));
        udpBufferSize += sizeof(dnsHeaderT);

        memcpy(udpBuffer + udpBufferSize, qname, strlen(qname));
        udpBufferSize += strlen(qname) + 1;

        memcpy(udpBuffer + udpBufferSize, &qinfo, sizeof(dnsQuestionT));
        udpBufferSize += sizeof(dnsQuestionT);

        if (sendto(socketFD, udpBuffer, udpBufferSize, 0,
            (struct sockaddr*)&destination, sizeof(destination)) < 0) {
            perror("Send failed");
            exit(1);
        }
        // logarea mesajului in hexa
        if (loggedMessage == 0) {
            loggedMessage = 1;
            LogMessageHexa(messageLog, udpBuffer, udpBufferSize);
        }

        if ((errorCodeVal = recvfrom(socketFD, udpBuffer, UDP_BUFFER_SIZE,
                0, (struct sockaddr *)&destination, &sockaddrSize)) < 0) {

            if (errorCodeVal == EAGAIN || errorCodeVal == EWOULDBLOCK) {
                // timeout on recv
                continue;
            }
            perror("Recieve failed");
            exit(1);
        }

        readDnsHeader = (dnsHeaderT*)udpBuffer;
        printf("Response from %s contains: \n", dnsServer);
        printf("%d answers\n\n",ntohs(readDnsHeader->ancount));
    } while (readDnsHeader->ancount == 0);

    fprintf(dnsLog, "; %s - %s %s\n\n", dnsServer, savedHostName, queryString);
    fprintf(dnsLog, ";; ANSWER SECTION:\n");

    nrOfAnswers = ntohs(readDnsHeader->ancount);
    readingOffset = 0;
    for (i = 0; i < nrOfAnswers; i++) {
        dnsRRT *dnsResponse;

        // + 2 pentru ca sarim peste NAME
        dnsResponse = (dnsRRT*)(udpBuffer + udpBufferSize + 2 + readingOffset);

        InterpretResponseData(query, ntohs(dnsResponse->rdlength), 
            (unsigned char*)(udpBuffer + udpBufferSize + 2 + sizeof(dnsRRT) + 
            readingOffset), extractedInfo, i, 
            (unsigned char*)udpBuffer);

        readingOffset += sizeof(dnsRRT) + 2 + ntohs(dnsResponse->rdlength);

        fprintf(dnsLog, "%s. IN %s %s\n", savedHostName, queryString, 
            extractedInfo);

        printf("Extracted info: %s\n\n", extractedInfo);
    }


    close(socketFD);
    printf("Socket closed!\n");
}

// interpreteaza raspunsul dns in functie de tipul de query
void InterpretResponseData(int query, int len, unsigned char *response,
    char *extractedInfo, int msgIndex, unsigned char *wholeMsg) {
    char miniBuffToCopy[128];
    int i;

    memset(extractedInfo, 0, QNAME_BUFFER_SIZE);

    if (query == A) {
        for (i = 0; i < len; i++) {

            sprintf(miniBuffToCopy, "%d", response[i]);
            strcat(extractedInfo, miniBuffToCopy);

            if (i != len - 1) {
                strcat(extractedInfo, ".");
            }
        }
    } else if (query == MX) {
        unsigned short mxPreferance;
        int currentReading = 0;

        // citim little endian
        memcpy(&mxPreferance, response + 1, 1);
        memcpy(((char*)&mxPreferance + 1), response, 1);

        sprintf(miniBuffToCopy, "%hu", mxPreferance);
        strcat(extractedInfo, miniBuffToCopy);
        strcat(extractedInfo, " ");


        currentReading = 2;
        // data labels cu possibili pointeri catre alta parte din mesaj
        ReadDataLabel(extractedInfo, (char*)&response[currentReading],
            (char*)wholeMsg, READ_DATA_CLASSIC);

    } else if (query == NS) {
        ReadDataLabel(extractedInfo, (char*)&response[0], (char*)wholeMsg, READ_DATA_CLASSIC);
    } else if (query == CNAME) {
        ReadDataLabel(extractedInfo, (char*)&response[0], (char*)wholeMsg, READ_DATA_CLASSIC);
    } else if (query == SOA) {
        unsigned int intReader;
        // primary name
        int currentReading = 0;
        currentReading += ReadDataLabel(extractedInfo, (char*)&response[0],
            (char*)wholeMsg, READ_DATA_CLASSIC);
        strcat(extractedInfo, " ");

        // mailbox
        currentReading += ReadDataLabel(extractedInfo, (char*)&response[currentReading],
            (char*)wholeMsg, READ_DATA_CLASSIC);
        strcat(extractedInfo, " ");

        // serial, refresh, retry, expiration, minimum.
        // toate acestea au o reprezentare de unsigned int in pachetul udp, deci sunt la fel
        for (i = 0; i < 5; i++) {
            memcpy((char*)&intReader + 3, (char*)&response[currentReading + 0], 1);
            memcpy((char*)&intReader + 2, (char*)&response[currentReading + 1], 1);
            memcpy((char*)&intReader + 1, (char*)&response[currentReading + 2], 1);
            memcpy((char*)&intReader + 0, (char*)&response[currentReading + 3], 1);

            sprintf(miniBuffToCopy, "%u", intReader);
            strcat(extractedInfo, miniBuffToCopy);
            if (i != 4) {
                currentReading += 4;
                strcat(extractedInfo, " ");
            }
        }
    } else if (query == TXT) {
        ReadDataLabel(extractedInfo, (char*)&response[0], (char*)wholeMsg, READ_DATA_ONLY_FIRST);
    } else if (query == PTR) {
        ReadDataLabel(extractedInfo, (char*)response - 1, (char*)wholeMsg, READ_DATA_ONLY_FIRST);
        // eliminam prima litera
        strcpy(miniBuffToCopy, extractedInfo + 1);
        strcpy(extractedInfo, miniBuffToCopy);

        char strLen = *(response - 1);
        for (i = 0; i < strLen - 1; i++) {
            if (extractedInfo[i] < 33 && extractedInfo[i] != 0) {
                extractedInfo[i] = '.';
            }
        }
    }
}

// citeste data labels. de asemenea, daca citeste un pointer de adresa pentru mesajul dns
// va citi recursiv de acolo
int ReadDataLabel(char *output, char *from, char *wholeMsg, int flag) {
    char nrOfCharsToRead;
    int currentReading = 0; // se incepe cititul de la indexul 0 din from
    int i;
    nrOfCharsToRead = from[currentReading]; // avem de citit atati octeti ca litere
    char miniBuffToCopy[128];

    // cat timp nr de octeti ce trebuie cititi nu este 0
    while (nrOfCharsToRead != 0) {
        // se face SI pe biti pentru a afla daca primi 2 biti sunt 11 (daca urmeaza o adresa)
        if ((nrOfCharsToRead & 0xc0) == 0xc0) {
            int resultedAddr;
            // 0x3f = 0b0011 1111 - eliminam primii 2 biti
            resultedAddr = (nrOfCharsToRead & 0x3f) * 255;
            resultedAddr += from[currentReading + 1];

            currentReading += 2; // am mai citit 2 octeti

            ReadDataLabel(output, &wholeMsg[resultedAddr], wholeMsg, flag);
            break;
        }

        for (i = 0; i < nrOfCharsToRead; i++) {
            // citim fiecare litera
            currentReading++;
            // o bagam in output
            sprintf(miniBuffToCopy, "%c", from[currentReading]);
            strcat(output, miniBuffToCopy);
        }
        currentReading++;
        nrOfCharsToRead = from[currentReading]; // nr urmator de octeti ce trebuie citit
        if (nrOfCharsToRead != 0 && flag != READ_DATA_ONLY_FIRST) {
            strcat(output, ".");
        }

        if (flag == READ_DATA_ONLY_FIRST) {
            break;
        }
    }
    return currentReading;
}

int main(int argc, char *argv[]) {
    FILE *dnsFile;

    if (argc < 3) {
       fprintf(stderr,"Folosire: \"%s (adresa IP / domeniu) (tip inregistrare)\"\n",
            argv[0]);
       exit(0);
    }

    dnsFile = fopen("dns_servers.conf", "rt");
    if (!dnsFile) {
        printf("Failed Opening dns servers\n");
        exit(0);
    }
    dnsLog = fopen("dns.log", "wt");
    messageLog = fopen("message.log", "wt");


    if (CheckIfPTR(argv[1], argv[2]) == -1) {
        printf("Invalid PTR argument\n");
        fclose(messageLog);
        fclose(dnsLog);
        fclose(dnsFile);
        return 0;
    }

    AskDNS(dnsFile, argv[1], GetQueryFromString(argv[2]), argv[2]);
    fclose(messageLog);
    fclose(dnsLog);
    fclose(dnsFile);
    return 0;
}

int GetNextDns(FILE *dnsFile, char *output) {

    memset(output, 0, DNS_NAME_BUFF_SIZE);
    while (fgets(output, DNS_NAME_BUFF_SIZE, dnsFile) != NULL) {
        if (output[0] == '#')
            continue;
        if (strlen(output) < 7)
            continue;

        if (output[strlen(output) - 1] == '\n') {
            output[strlen(output) - 1] = 0;
        }
        return 1;
    }
    return -1;
}

// google.com -> 6google3com
void ConvertToDnsName(char *hostName, char *dnsName) {
    int i, len;
    const char s[2] = ".";
    char *token;

    i = 0;

    token = strtok(hostName, s);

    while(token != NULL) {
        len = strlen(token);
        dnsName[i] = len;
        i++;
        strcpy(dnsName + i, token);
        i+= len;

        token = strtok(NULL, s);
    }
}

// verifica daca este un argument valid ip-ul dat in PTR si converteste adresa
// 8.8.4.4 -> 4.4.8.8.in-addr.arpa
int CheckIfPTR(char *possibleIP, char *queryString) {
    if (strcmp(queryString, "PTR") == 0) {
        char ipVals[16][32];
        int i, j, len;
        const char s[2] = ".";
        char *token;
        char miniBuffToCopy[128];

        i = 0;
        token = strtok(possibleIP, s);
        while (token != NULL) {
            strcpy(ipVals[i], token);

            len = strlen(ipVals[i]);
            // verificare ca literele sa fie doar numere ascii
            for (j = 0; j < len; j++) {
                if (ipVals[i][j] < 48 || ipVals[i][j] > 57) {
                    return -1;
                }
            }

            i++;
            token = strtok(NULL, s);
        }
        len = i;
        if (len != 4) {
            return -1;
        }

        strcpy(miniBuffToCopy, ipVals[len - 1]);
        strcat(miniBuffToCopy, ".");
        for (i = len - 2; i >= 0; i--) {
            strcat(miniBuffToCopy, ipVals[i]);
            strcat(miniBuffToCopy, ".");
        }
        strcat(miniBuffToCopy, "in-addr.arpa");
        strcpy(possibleIP, miniBuffToCopy);

        return 1;
    }
    return 0;
}

int GetQueryFromString(char *str) {
    if (strcmp(str, "A") == 0) {
        return A;
    } else if (strcmp(str, "MX") == 0) {
        return MX;
    } else if (strcmp(str, "NS") == 0) {
        return NS;
    } else if (strcmp(str, "CNAME") == 0) {
        return CNAME;
    } else if (strcmp(str, "SOA") == 0) {
        return SOA;
    } else if (strcmp(str, "TXT") == 0) {
        return TXT;
    } else {
        return PTR;
    }
}

void LogMessageHexa(FILE *where, char *data, int nrOfBytes) {
    int i;
    for (i = 0; i < nrOfBytes; i++) {
        if (data[i] < 16) {
            //fprintf(where, "0");
        }
        fprintf(where, "%02X ", (unsigned char)data[i]);
        if (i % 2 == 1) {
            fprintf(where, " ");
        }

        if (i % 4 == 3) {
            fprintf(where, "   ");
        }
        if (i % 8 == 7) {
            fprintf(where, "\n");
        }
    }
    fprintf(where, "\n");
}
