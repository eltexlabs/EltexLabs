//// 25.10.19
//// LAB #12 - server
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

#include "shared.h"	// Header srared with clients



// Definitions and typedefs //
#define SERV_QSZ	5	// Server queue size
#define SERV_TLOOP	1

// Prototypes //


// Main code //

int main (int argc, char * argv[])
{
	// Server main func //

	int num_clients, max_clients;
	int tcp_port, udp_port;
	int sv_sock, *cl_socks, udp_sock, temp_sock;
	struct sockaddr_in sv_addr, cl_addr, udp_addr;
	int cl, m; //counter, // Counters
	int result;
	char buf[BUF_SZ];
	char * msq[SV_MSQ_SZ];
	int num_msq;
	
	puts("Server: starting up ...\n");
	
	// Get args
	if (argc != 4)
	{
		puts("Ivalid args");
		puts("Server usage: [prog] [tcp_port] [udp_port] [max_clients] ");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &tcp_port);
	sscanf(argv[2], "%d", &udp_port);
	sscanf(argv[3], "%d", &max_clients);
	if (max_clients < 1)
		max_clients = 1;
	
	// Alloc mem
	cl_socks = calloc( max_clients, sizeof(*cl_socks) );
	
	// Prepare TCP client sockets
	num_clients = 0;
	for (cl = 0; cl < max_clients; cl++)
		cl_socks[cl] = -1;
	
	// Prepare UDP server socket
	puts("Server: opening UDP socket ...");
	PrepInetServerSock(NULL, udp_port, &udp_sock, &udp_addr, true, SERV_QSZ);
	
	// Prepare TCP server socket
	puts("Server: opening TCP socket ...");
	PrepInetServerSock(NULL, tcp_port, &sv_sock, &sv_addr, false, SERV_QSZ);
	fcntl(sv_sock, F_SETFL, O_NONBLOCK);
	
	// Main loop
	num_msq = 0;
	//counter = 0;
	while (!UserQuit())
	{
		// Check connections
		// Check message queue
			// not full: Recieve incoming data (select(), peek())
				// still not full: Send UDP for cl1
			// full: send messages
				// send UDP for cl2
		
		#ifdef DEBUG
		puts("Checking incoming connections ...");
		#endif
		while (1)
		{
			// Check for incoming connections
			socklen_t socklen = sizeof(cl_addr);
			temp_sock = accept(sv_sock, (struct sockaddr *) &cl_addr, &socklen);
			if (temp_sock != FAIL)
			{
				printf("Client connected (0x%X, %d)\n", htonl(cl_addr.sin_addr.s_addr), htons(cl_addr.sin_port));
				
				// Check number of connections
				if (num_clients < max_clients)
				{
					// Accept
					for (cl = 0; cl < max_clients; cl++)
						if (cl_socks[cl] == -1)
							break;
					cl_socks[cl] = temp_sock;
					fcntl(cl_socks[cl], F_SETFL, O_NONBLOCK);
					num_clients++;
				}
				else
				{
					// Reject
					puts("Too many clients, dropping connection");
					close(temp_sock);
				}
			}
			else
			{
				#ifdef DEBUG
				puts("No connections");
				#endif
				break;
			}
		}
		
		// Check message queue
		if (num_msq < SV_MSQ_SZ)
		{
			// Send UDP request to client
			//if (counter < 2)
			//	counter++;
			//else
			//{
			//	counter = 0;
				
				puts("Sending UDP broadcast to clients #1 ...");
				
				if (sendto(udp_sock, MSG_ECHO1, sizeof(MSG_ECHO1), 0, (struct sockaddr *) &udp_addr,
				 sizeof(udp_addr)) != sizeof(MSG_ECHO1))
				{
					puts("sendto() error");
					exit(1);
				}
			//}
			
			// Check response
			#ifdef DEBUG
			puts("Checking TCP ...");
			#endif
			strcpy(buf, "");
			for (cl = 0; cl < max_clients; cl++)
			{
				// Skip non-connected sockets
				if (cl_socks[cl] == -1)
					break;
				
				// Recieve data
				result = recvfrom(cl_socks[cl], buf, sizeof(buf), 0, (struct sockaddr *) NULL, NULL); //MSG_DONTWAIT
				if (result != FAIL)
				{
					if (result != 0)
					{
						printf("Got message from client: %s \n", buf);
						
						// Add message to queue
						msq[num_msq] = AllocString(buf);
						num_msq++;
						if (num_msq >= SV_MSQ_SZ)
							break;
					}
					else
					{
						// Disconnected - close socket and mark as non-connected
						close(cl_socks[cl]);
						cl_socks[cl] = -1;
						num_clients--;
						puts("Client disconnected ...");
					}
				}
				else
				{
					#ifdef DEBUG
					puts("Nothing on TCP");
					#endif
				}
			}
		}
		else
		{
			for (m = 0; m < num_msq; m++)
			{
				puts("Sending UDP broadcast to clients #2 ...");
				if (sendto(udp_sock, MSG_ECHO2, sizeof(MSG_ECHO2), 0, (struct sockaddr *) &udp_addr,
				 sizeof(udp_addr)) != sizeof(MSG_ECHO2))
				{
					puts("sendto() error");
					exit(1);
				}
				
				// Send all messages for each client
				puts("Sending TCP messages ...");
				for (m = 0; m < num_msq; m++)
				{
					for (cl = 0; cl < max_clients; cl++)
						if (cl_socks[cl] != -1)
							send(cl_socks[cl], msq[m], strlen(msq[m])+1, 0);
				}
			}
			// Clear queue
			for (m = 0; m < num_msq; m++)
				FreeString(msq[m]);
			num_msq = 0;
		}
		
		sleep(1);
	}
	
	// Close sockets
	close(udp_sock);
	close(sv_sock);
	for (cl = 0; cl < max_clients; cl++)
		if (cl_socks[cl] != -1)
			close(cl_socks[cl]);
	
	// Free memory
	free(cl_socks);
	
	puts("Server: shutting down\n");
	return EXIT_SUCCESS;
}
