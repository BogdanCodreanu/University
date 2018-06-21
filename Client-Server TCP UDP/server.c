#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 2
#define SUCCESS 0
#define FAILURE 1
#define BUFFER_SIZE 512

void ServerError(char* msg) {
	perror(msg);
}

int RunConnection(int tcpSocketDescriptor, int udpSocketDescriptor);
int SaveSocket(int clientFDS, int *clientsFDS);

int RunServer(int portNumber) {
	int udpSocketDescriptor;
	int tcpSocketDescriptor;
	socklen_t sockaddrSize;
	struct sockaddr_in serverSocketAddr;
	int tcpSocketOption = 1;


	sockaddrSize = sizeof(struct sockaddr);

	// initializare descriptori upd si tcp
	udpSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocketDescriptor == -1) {
		ServerError("UDP: Creating socket error");
    	return FAILURE;
	}
	tcpSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpSocketDescriptor == -1) {
		ServerError("TCP: Creating socket error");
    	return FAILURE;
	}

	// initializarea socket server
	memset((char*)&serverSocketAddr, 0, sizeof(serverSocketAddr));
	serverSocketAddr.sin_family = AF_INET;
    serverSocketAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //htonl(INADDR_ANY);
    serverSocketAddr.sin_port = htons(portNumber);

    // reusing tcp address
    setsockopt(tcpSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &tcpSocketOption, sizeof(tcpSocketOption));

    // binding sockets
    if (bind(udpSocketDescriptor, (struct sockaddr*)&serverSocketAddr, sockaddrSize) < 0) {
    	ServerError("UDP: Failed on binding socket");
    	return FAILURE;
    }
    if (bind(tcpSocketDescriptor, (struct sockaddr*)&serverSocketAddr, sockaddrSize) < 0) {
    	ServerError("TCP: Failed on binding socket");
    	return FAILURE;
    }

    if (listen(tcpSocketDescriptor, MAX_CLIENTS) < 0) {
    	ServerError("TCP: Listening error");
    	return FAILURE;
    }
    RunConnection(tcpSocketDescriptor, udpSocketDescriptor);

    close(udpSocketDescriptor);
    close(tcpSocketDescriptor);
    return SUCCESS;
}

int RunConnection(int tcpSocketDescriptor, int udpSocketDescriptor) {
	fd_set readFDS;
	char incomingBuffer[BUFFER_SIZE] = { 0 };
	socklen_t sockaddrSize = sizeof(struct sockaddr);

	// client sockets
	struct sockaddr_in clientSocketAddr;
	int clientsFDS[MAX_CLIENTS] = { 0 };
	int clientFDS;

	FD_ZERO(&readFDS);
	FD_SET(tcpSocketDescriptor, &readFDS);

	
	while (1) {
		int i;
		FD_ZERO(&readFDS);
		FD_SET(tcpSocketDescriptor, &readFDS);
		FD_SET(udpSocketDescriptor, &readFDS);
		int bytesRead;
		int maxFDS;

		maxFDS = tcpSocketDescriptor > udpSocketDescriptor ? tcpSocketDescriptor : udpSocketDescriptor;

		for (i = 0; i < MAX_CLIENTS; i++) {
			if (clientsFDS[i] > 0) {
				FD_SET(clientsFDS[i], &readFDS);
			}
			if (clientsFDS[i] > maxFDS) {
				maxFDS = clientsFDS[i];
			}
		}

		if (select(maxFDS + 1, &readFDS, NULL, NULL, NULL) < 0) {
			ServerError("Error on selecting sockets");
			return FAILURE;
		}

		// tcp incoming connections
		if (FD_ISSET(tcpSocketDescriptor, &readFDS)) {
			clientFDS = accept(tcpSocketDescriptor, (struct sockaddr*)&clientSocketAddr, &sockaddrSize);
			if (clientFDS < 0) {
				ServerError("TCP: Failed to accept socket");
				continue;
			}
			printf("TCP: Connection established on FDS: %d\n", clientFDS);
			if (SaveSocket(clientFDS, clientsFDS) < 0) {
				ServerError("TCP: Failed to register socket. Limit could be exceeded.");
			}
		}

		// messages from tcp clients
		for (i = 0; i < MAX_CLIENTS; i++) {
			clientFDS = clientsFDS[i];
			if (FD_ISSET(clientFDS, &readFDS)) {
    			memset(incomingBuffer, 0, BUFFER_SIZE);
    			
				bytesRead = recv(clientFDS, incomingBuffer, BUFFER_SIZE, 0);
				if (bytesRead < 0) {
					ServerError("TCP: Receving message failed.");
					continue;
				} else if (bytesRead == 0) {
					printf("TCP: Client on socket %d disconnected\n", clientFDS);
                    close(clientFDS);
                    clientsFDS[i] = 0;
                    continue;
				}
				printf("TCP: Recieved message from client %d: %s\n", clientFDS, incomingBuffer);
			}
		}

		// udp incoming messages
		if (FD_ISSET(udpSocketDescriptor, &readFDS)) {
			memset(incomingBuffer, 0, BUFFER_SIZE);
			bytesRead = recvfrom(udpSocketDescriptor, incomingBuffer, BUFFER_SIZE,
				0, (struct sockaddr*)&clientSocketAddr, &sockaddrSize);

			if (bytesRead < 0) {
				ServerError("UDP: Error on receiving message");
				continue;
			}
			printf("UDP: Recieved message from udp: %s\n", incomingBuffer);
		}
	}
}

int SaveSocket(int clientFDS, int *clientsFDS) {
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clientsFDS[i] == 0) {
			clientsFDS[i] = clientFDS;
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[]) {
	int portNumber;

    if (argc < 3) {
       fprintf(stderr,"Usage: \"%s server_port users_data_file\"\n", argv[0]);
       exit(0);
    }

    portNumber = atoi(argv[1]);
    RunServer(portNumber);
	return 0;
}
