//// 25.10.19
//// LAB #12 - client #1
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

#include "shared.h"	// Header shared with server



// Definitions and typedefs //


// Prototypes //


// Main code //

int main (int argc, char * argv[])
{
	int tcp_port, udp_port;
	int cl_sock, udp_sock;
	socklen_t sock_sz;	// for sendto()/recvfrom()
	struct sockaddr_in cl_addr, sv_addr, udp_addr;
	int result;
	int try;
	int sltime;
	char * address;
	char buf[BUF_SZ];
	char * strings[NUM_STRINGS] = {STRINGS};
	bool quit;
	bool auth;
	int auth_counter;
	fd_set fdset;
	struct timeval tout;
	
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
	
	// Init randomizer
	srand( time(NULL) + getpid() );
	
	// Prepare TCP client socket
	puts("Client: opening TCP socket ...");
	PrepInetClientSock(address, tcp_port, &cl_sock, &cl_addr, false);
	//shutdown(cl_sock, SHD_RD);	// Write only
	fcntl(cl_sock, F_SETFL, O_NONBLOCK);
	
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
	quit = false;
	auth = false;
	auth_counter = 0;
	while (!UserQuit() && !quit)
	{
		// Clear fd set and prepare timeout for select()
		FD_ZERO(&fdset);	// set should be reset before each select() call
		tout.tv_sec = 1;	// timeout should be reset before each select() call
		tout.tv_usec = 0;
		
		if (!auth)
		{
			// Not authorized on the server //
			
			if (auth_counter == 0)
			{
				// Send auth request once per N cycles
				puts("Sending auth request ...");
				send(cl_sock, MSG_AUTH1, sizeof(MSG_AUTH1), 0);
			}
			else
			{
				// Wait for something to arrive on TCP socket
				FD_SET(cl_sock, &fdset);
				result = select(cl_sock+1, &fdset, NULL, NULL, &tout);
				COND_EXIT(result == FAIL, "select() error");
				
				// Check auth response
				puts("Checking auth response ...");
				result = recv(cl_sock, buf, sizeof(buf), 0);
				if (result == 0)
				{
					puts("Server is out of reach");
					quit = true;
				}
				else if (result != FAIL)
				{
					if (!strcmp(buf, MSG_AUTH_ACK))
					{
						puts("Authenticated");
						auth = true;
					}
					else if (!strcmp(buf, MSG_AUTH_DENY))
					{
						puts("Authentication failed");
						quit = true;
					}
					else
					{
						#ifdef DEBUG
						puts("Bad auth message from server");
						#endif
					}
				}
			}
			
			if (++auth_counter > 10)
				auth_counter = 0;
		}
		else
		{
			// Authorized on server //
			
			#ifdef DEBUG
			puts("Checking UDP ...");
			#endif
			//puts("Waiting for UDP ...");
			
			// Wait for something to arrive on UDP socket
			FD_SET(udp_sock, &fdset);
			result = select(udp_sock+1, &fdset, NULL, NULL, &tout);
			COND_EXIT(result == FAIL, "select() error");
			if ( !FD_ISSET( udp_sock, &fdset) )
				continue;	// Nothing on UDP - restart loop
			
			// Check out UDP
			sock_sz = sizeof(udp_addr);
			result = recvfrom(udp_sock, buf, sizeof(buf), 0, (struct sockaddr *) &udp_addr, &sock_sz); //MSG_DONTWAIT
			if (result != FAIL)
			{
				if (!strcmp(buf, MSG_ECHO1))
				{
					puts("Got UDP echo from server ...");
					sltime = (rand() % MAX_SLEEP)+1;
					printf("Going to sleep for %d second(s)\n", sltime);
					sleep(sltime);
					puts("Woke up ...");
					
					result = rand() % (NUM_STRINGS);
					printf(">>> Sending response: %s \n", strings[result]);
					strcpy(buf, strings[result]);
					send(cl_sock, buf, strlen(buf)+1, 0);
				}
			}
			else
			{
				#ifdef DEBUG
				puts("Nothing on UDP");
				#endif
			}
		}
		
		// Wait for next cycle
		//sleep(1);
	}
	
	// Close sockets
	close(cl_sock);
	
	puts("Client: shutting down ...\n");
}
