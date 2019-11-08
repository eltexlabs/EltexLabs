//// 25.10.19
//// LAB #12 - shared funcs
////
//// Student: Alexey Leushin
////

// Includes //
#include <stdio.h>			// printf, puts, scanf, gets
#include <string.h>			// strlen, strcpy, memset
#include <stdlib.h>			// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>		// System types
#include <wait.h>			// waitpid
#include <unistd.h>			// sleep, fork, getpid, close, unlink
#include <time.h>			// time
#include <pthread.h>		// pthread_*** functions
#include <sys/socket.h>		// Sockets
#include <netinet/in.h>		// Inet
#include <arpa/inet.h>		// Inet
//#include <sys/un.h>		// Unix

#include "shared.h"



// Funcs //

void PrepInetSock(const char * address, int port, int * sock, void * sockaddr, bool udp)
{
	int type, optarg, result;
	struct sockaddr_in *saddr;
	uint32_t ipaddress;
	
	// Prep
	saddr = (struct sockaddr_in *) sockaddr;
	if (udp)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;
	COND_EXIT(port < 0 || port > 0xFFFF, "Invalid port configuration!");
	
	// Make socket
	*sock = socket(AF_INET, type, 0);
	COND_EXIT(*sock == FAIL, "socket() error");
	
	// Config socket
	if (udp)
	{
		optarg = 1;
		result = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &optarg, sizeof(optarg));	// Multiple local UDP clients
		COND_EXIT(result == FAIL, "setsockopt() error");
		result = setsockopt(*sock, SOL_SOCKET, SO_BROADCAST, &optarg, sizeof(optarg));	// Enable UDP broadcast
		COND_EXIT(result == FAIL, "setsockopt() error");
	}
	//result = setsockopt(*sock, SOL_SOCKET, SO_RCVLOWAT, (char *)&msg_len, sizeof(msg_len));
	
	// Prep address
	if (address)
	{
		ipaddress = inet_addr(address);
		COND_EXIT(ipaddress == FAIL, "inet_addr() error");
	}
	else
		ipaddress = htonl(INADDR_ANY);
	memset(saddr, 0x00, sizeof(*saddr));	// Addr: Clear struct
    saddr->sin_family = AF_INET;			// Addr: Internet
    saddr->sin_addr.s_addr = ipaddress;		// Addr: IP address
    saddr->sin_port = htons(port);			// Addr: Port number
    
    // Bind socket
	result = bind(*sock, (struct sockaddr *) saddr, sizeof(*saddr));
	COND_EXIT(result == FAIL, "bind() error");
	printf("%s socket opened (port %d)\n", udp? "UDP" : "TCP", port);
	if (address)
		printf("socket address is %s (0x%X)\n", address, htonl(ipaddress));
}

int UserQuit()
{
	static bool firstcall = true;
	fd_set fds;
	struct timeval tout;
	char buf[10];
	
	// Notify user
	if (firstcall)
	{
		firstcall = false;
		puts("[Press q+enter to quit]");
	}
	
	// stdio fd is 0
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	
	// Set timeout
	tout.tv_sec = 0;
	tout.tv_usec = 0;
	
	// Wait for stdio
	if (select(1, &fds, NULL, NULL, &tout))
	{
		fgets(buf, sizeof(buf), stdin);
		if ( !strcmp("q\n", buf) )
			return 1;
	}
	
	return 0;
}

//void msg_header_init(msg_base_t * header, , uint32_t datasize)
