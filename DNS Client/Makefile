all: build

build: dnsclient

dnsclient: dnsclient.c
	gcc dnsclient.c -Wall -o dnsclient

run: build
	./dnsclient google.com MX

clean:
	rm dnsclient *.log