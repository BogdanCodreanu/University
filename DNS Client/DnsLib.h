#ifndef DNS_LIB
#define DNS_LIB

#define A 1
#define NS 2
#define CNAME 5
#define MX 15
#define SOA 6
#define TXT 16
#define PTR 12

typedef struct __attribute__((packed)) {
	unsigned short id;

	unsigned char rd :1;
	unsigned char tc :1;
	unsigned char aa :1;
	unsigned char opcode :4;
	unsigned char qr :1;

	unsigned char rcode :4;
	unsigned char z :3;
	unsigned char ra :1;

	unsigned short qdcount;
	unsigned short ancount;
	unsigned short nscount;
	unsigned short arcount;
} dnsHeaderT;

typedef struct __attribute__((packed)) {
	unsigned short qtype;
	unsigned short qclass;
} dnsQuestionT;

typedef struct __attribute__((packed)) {
	unsigned short type;
	unsigned short _class;
	unsigned int tt1;
	unsigned short rdlength;
} dnsRRT;

dnsHeaderT CreateDns(unsigned short id) {
	dnsHeaderT dns;

	dns.id = id;

	dns.rd = 1;
	dns.tc = 0;
	dns.aa = 0;
	dns.opcode = 0;
	dns.qr = 0;

	dns.rcode = 0;
	dns.z = 0;
	dns.ra = 0;

	dns.qdcount = htons(1);
	dns.ancount = 0;
	dns.nscount = 0;
	dns.arcount = 0;

	return dns;
}

#endif
