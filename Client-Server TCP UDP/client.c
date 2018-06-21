#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SUCCESS 0
#define FAILURE 1
#define BUFFER_SIZE 512

void ServerError(char* msg) {
	perror(msg);
}

int ConnectToServer(char *ipAddress, int portNumber) {
	int udpSocketDescriptor;
	int tcpSocketDescriptor;
	struct sockaddr_in serverSocketAddr;
	char inputBuffer[BUFFER_SIZE];

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

	// initializare socket server
	//memset((char*)&serverSocketAddr, 0, sizeof(serverSocketAddr));
	serverSocketAddr.sin_family = AF_INET;
    serverSocketAddr.sin_port = htons(portNumber);
    inet_aton(ipAddress, &serverSocketAddr.sin_addr);

    if (connect(tcpSocketDescriptor, (struct sockaddr*)&serverSocketAddr, sizeof(serverSocketAddr)) < 0) {
    	ServerError("TCP: Connection failed");
	    close(udpSocketDescriptor);
	    close(tcpSocketDescriptor);
		return FAILURE;
    }

    printf("TCP: Connection Established!\n");

    while (1) {
    	printf("TCP message: ");
    	memset(inputBuffer, 0, BUFFER_SIZE);
    	fgets(inputBuffer, BUFFER_SIZE - 1, stdin);

    	if (send(tcpSocketDescriptor, inputBuffer, strlen(inputBuffer), 0) < 0) {
    		ServerError("TCP: Error writing to socket.");
    		printf("Connection Stopped\n");
    		break;
    	}

    	printf("UDP message: ");
    	memset(inputBuffer, 0, BUFFER_SIZE);
    	fgets(inputBuffer, BUFFER_SIZE - 1, stdin);
    	if (sendto(udpSocketDescriptor, inputBuffer, strlen(inputBuffer) + 1, 0,
    		(struct sockaddr *)&serverSocketAddr, sizeof(struct sockaddr)) < 0) {
    		ServerError("UDP: Error while sending to server");
    		printf("Connection Stopped\n");
    		break;
    	}
    }

    close(udpSocketDescriptor);
    close(tcpSocketDescriptor);
	return SUCCESS;
}

int main(int argc, char *argv[]) {
	int portNumber;
    if (argc < 3) {
       fprintf(stderr,"Usage: \"%s server_IP server_port\"\n", argv[0]);
       exit(0);
    }
    portNumber = atoi(argv[2]);
    ConnectToServer(argv[1], portNumber);
	return 0;
}
