//// 25.10.19
//// LAB #12 - client #2
////
//// Student: Alexey Leushin
////



// Includes //
#include <stdio.h>		// printf, puts, scanf, gets
#include <string.h>		// strlen, strcpy, memset
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// System types
#include <wait.h>		// waitpid
#include <unistd.h>		// sleep, fork, getpid, close, unlink
//#include <time.h>		// time
#include <sys/time.h>
#include <pthread.h>	// pthread_*** functions
#include <sys/socket.h>			// Sockets
#include <netinet/in.h>			// Inet
//#include <sys/un.h>			// Unix

#include <errno.h>
#include <fcntl.h>

#include "shared.h"	// Header srared with server



// Definitions and typedefs //


// Prototypes //


// Main code //

int main (int argc, char * argv[])
{
	int tcp_port, udp_port;
	int cl_sock, udp_sock; // sv_sock,
	struct sockaddr_in cl_addr, sv_addr, udp_addr;
	int result;//, optarg;
	int try;
	char * address;
	
	puts("Client: starting up ...\n");
	
	// Get args
	if (argc != 4)
	{
		puts("Ivalid args");
		puts("Server usage: [prog] [server_addr] [tcp_port] [udp_port]");
		exit(EXIT_SUCCESS);
	}
	address = argv[1];
	sscanf(argv[2], "%d", &tcp_port);
	sscanf(argv[3], "%d", &udp_port);
	
	// Prepare TCP client socket
	puts("Client: opening TCP socket ...");
	PrepInetClientSock(address, tcp_port, &cl_sock, &cl_addr, false);
	shutdown(cl_sock, SHD_WR);	// Write only
	
	// Prepare UDP client socket
	puts("Client: opening UDP socket ...");
	PrepInetClientSock(NULL, udp_port, &udp_sock, &udp_addr, true);
	fcntl(udp_sock, F_SETFL, O_NONBLOCK);
	
	// Prepare server address
	memset(&sv_addr, 0x00, sizeof(struct sockaddr));	// Clean struct
	
	// Connect to server via TCP
	for (try = 0; try < CL_TRYCOUNT; try++)
	{
		sleep(1);	// Period between attempts
		
		result = connect(cl_sock, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
		if (result == FAIL)
			printf("Client: connect attempt #%d failed \n", try);
		else
		{
			printf("Connected to server (0x%x, %d)\n", htonl(cl_addr.sin_addr.s_addr), htons(cl_addr.sin_port));
			break;
		}
	}
	COND_EXIT(result == FAIL, "connect() error");
	
	// Main loop
	/*fd_set fds;
	struct timeval tout;
	FD_ZERO(&fds);
	FD_SET(udp_sock, &fds);
	tout.tv_sec = 2;
	tout.tv_usec = 0;*/
	while (!UserQuit())
	{
		// Check out UDP
		char buf[300];
		socklen_t sz = sizeof(udp_addr);
		
		puts("Checking UDP ...");
		result = recvfrom(udp_sock, buf, sizeof(buf), 0, (struct sockaddr *) &udp_addr, &sz); //MSG_DONTWAIT
		if (result != FAIL)
		{
			if (!strcmp(buf, "udp2"))
			{
				puts("Got UDP echo from server ...");
				result = recv(cl_sock, buf, sizeof(buf), 0);
				if (result != FAIL)
				{
					puts("TCP message from server:");
					puts(buf);
				}
			}
		}
		else
			puts("Nothing on UDP");
		
		sleep(1);
	}
	
	// Close sockets
	close(cl_sock);
	
	puts("Client: shutting down ...\n");
}
