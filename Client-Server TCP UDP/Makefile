all: client server

client: client.c
	gcc client.c -lnsl -Wall -o client

server: server.c BankingControl
	gcc server.c BankingControl.o -lnsl -Wall -o server

BankingControl: BankingControl.c
	gcc -g -c BankingControl.c

runServer: all
	./server 5000 userTestFile

runClient: all
	./client 127.0.0.1 5000

clean:
	rm -f server client *.o