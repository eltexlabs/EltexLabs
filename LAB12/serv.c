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

typedef enum cltype_t
{
	CL_UNKNOWN = -1,
	CL_EMITTER = 0,
	CL_WORKER = 1,
} cltype_t;

typedef struct client_t
{
	int sock;
	cltype_t type;
} client_t;

// Prototypes //


// Main code //

int main (int argc, char * argv[])
{
	// Server main func //

	int num_clients, max_clients;
	int tcp_port, udp_port;
	int sv_sock, udp_sock, temp_sock; // *cl_socks, 
	client_t * clients;
	struct sockaddr_in sv_addr, cl_addr, udp_addr;
	int cl; //, m; //counter, // Counters
	int result;
	char buf[BUF_SZ];
	char * msq[SV_MSG_QSZ];
	int num_msq;
	fd_set fdset;
	struct timeval tout;
	int fdmax;
	
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
	clients = calloc( max_clients, sizeof(*clients) );
	
	// Prepare TCP client sockets
	num_clients = 0;
	for (cl = 0; cl < max_clients; cl++)
	{
		clients[cl].sock = -1;
		clients[cl].type = CL_UNKNOWN;
	}
	
	// Prepare UDP server socket
	puts("Server: opening UDP socket ...");
	PrepInetServerSock(NULL, udp_port, &udp_sock, &udp_addr, true, SV_SOCK_QSZ);
	
	// Prepare TCP server socket
	puts("Server: opening TCP socket ...");
	PrepInetServerSock(NULL, tcp_port, &sv_sock, &sv_addr, false, SV_SOCK_QSZ);
	fcntl(sv_sock, F_SETFL, O_NONBLOCK);
	
	// Main loop
	num_msq = 0;
	//int counter = 0;
	while (!UserQuit())
	{
		// Check connections
		// Check message queue
			// not full: Recieve incoming data (select(), peek())
				// still not full: Send UDP for cl1
			// full: send messages
				// send UDP for cl2
		
		
		// Check if message queue is not full
		//counter++;
		if (num_msq < SV_MSG_QSZ) // && (counter % 10) == 0)
		{
			// Send UDP request to clients of type 1
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
		}
		
		// Prepare fd set and timeout for select()
		FD_ZERO(&fdset);	// set should be reset before each select() call
		FD_SET(sv_sock, &fdset);
		fdmax = sv_sock;
		for (cl = 0; cl < max_clients; cl++)
		{
			if (clients[cl].sock != -1)
			{
				FD_SET(clients[cl].sock, &fdset);
				if (fdmax < clients[cl].sock)	// Determine biggest fd
					fdmax = clients[cl].sock;
			}
		}
		tout.tv_sec = 1;	// timeout should be reset before each select() call
		tout.tv_usec = 0;
		
		// Wait for something to happen in given time
		result = select(fdmax+1, &fdset, NULL, NULL, &tout);	// Biggest fd + 1
		COND_EXIT(result == FAIL, "select() error");
		
		// Check incoming connections
		if ( FD_ISSET(sv_sock, &fdset) )
		{
			#ifdef DEBUG
			puts("Checking incoming connections ...");
			#endif
			// Loop until all incoming connections are checked
			while (1)
			{
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
							if (clients[cl].sock == -1)
								break;
						clients[cl].type = CL_UNKNOWN;
						clients[cl].sock = temp_sock;
						fcntl(clients[cl].sock, F_SETFL, O_NONBLOCK);
						//FD_SET(clients[cl].sock, &fdset);
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
		}
		
		// Check TCP responses
		#ifdef DEBUG
		puts("Checking TCP ...");
		#endif
		strcpy(buf, "");
		for (cl = 0; cl < max_clients; cl++)
		{
			// Skip client entries with non-connected sockets and sockets without FD_SET
			if (clients[cl].sock == -1)
				continue;
			if ( !FD_ISSET( clients[cl].sock, &fdset) )
				continue;
			
			// Recieve data
			result = recvfrom(clients[cl].sock, buf, sizeof(buf), 0, (struct sockaddr *) NULL, NULL); //MSG_DONTWAIT
			if (result != FAIL)
			{
				if (result != 0)
				{
					// Check client type
					if (clients[cl].type == CL_EMITTER)
					{
						// Emitter sent new workload //
						
						if (num_msq < SV_MSG_QSZ)
						{
							// Add message to queue
							printf("Got message from client: %s \n", buf);
							msq[num_msq] = AllocString(buf);
							num_msq++;
							if (num_msq >= SV_MSG_QSZ)
								break;
						}
					}
					else if (clients[cl].type == CL_WORKER)
					{
						// Worker sent something? //
						printf("Got message from worker: %s \n", buf);
					}
					else
					{
						// Should be authentication message //
						if ( !strcmp(buf, MSG_AUTH1) )
						{
							clients[cl].type = CL_EMITTER;
							send(clients[cl].sock, MSG_AUTH_ACK, sizeof(MSG_AUTH_ACK), 0);
							puts("Client of type 1 (emitter) authenticated");
						}
						else if ( !strcmp(buf, MSG_AUTH2) )
						{
							clients[cl].type = CL_WORKER;
							send(clients[cl].sock, MSG_AUTH_ACK, sizeof(MSG_AUTH_ACK), 0);
							puts("Client of type 2 (worker) authenticated");
						}
						else
						{
							// Bad auth? //
							printf("Bad auth message: %s \n", buf);
						}
					}
				}
				else
				{
					// Disconnected - close socket and mark as non-connected
					//FD_CLR(clients[cl].sock, &fdset);
					close(clients[cl].sock);
					clients[cl].sock = -1;
					clients[cl].type = CL_UNKNOWN;
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
		
		// Check if message queue is not empty
		if (num_msq > 0)
		{
			//for (m = 0; m < num_msq; m++)
			//{
				puts("Sending UDP broadcast to clients #2 ...");
				if (sendto(udp_sock, MSG_ECHO2, sizeof(MSG_ECHO2), 0, (struct sockaddr *) &udp_addr,
				 sizeof(udp_addr)) != sizeof(MSG_ECHO2))
				{
					puts("sendto() error");
					exit(1);
				}
				
				// Send messages to clients of type 2 (if present)
				for (cl = 0; cl < max_clients; cl++)
				{
					if (clients[cl].sock != -1 && clients[cl].type == CL_WORKER)
					{
						// Send last message and remove it from queue
						num_msq--;
						result = send(clients[cl].sock, msq[num_msq], strlen(msq[num_msq])+1, MSG_NOSIGNAL);
						//printf("========= Send result: %d \n", result);
							// MSG_NOSIGNAL - prevent unhandled SIGPIPE terminating process
						if (result == FAIL)
						{
							// Client disconnected, skip
							puts("Client dropped connection ...");
							//FD_CLR(clients[cl].sock, &fdset);
							close(clients[cl].sock);
							clients[cl].sock = -1;
							clients[cl].type = CL_UNKNOWN;
							num_clients--;
							num_msq++;
							continue;
						}
						FreeString(msq[num_msq]);
						
						// Check if queue is empty
						if (num_msq == 0)
							break;
					}
				}
				
				#ifdef DEBUG
				puts("Messages sent to workers ...");
				#endif
				/*// Send all messages for each client of type 2
				puts("Sending TCP messages ...");
				for (m = 0; m < num_msq; m++)
				{
					for (cl = 0; cl < max_clients; cl++)
						if (clients[cl].sock != -1 && clients[cl].type == CL_WORKER)
							send(clients[cl].sock, msq[m], strlen(msq[m])+1, 0);
				}*/
			//}
			/*// Clear queue
			for (m = 0; m < num_msq; m++)
				FreeString(msq[m]);
			num_msq = 0;*/
		}
		
		// Wait for the next cycle
		//sleep(1);
	}
	
	// Close sockets
	close(udp_sock);
	close(sv_sock);
	for (cl = 0; cl < max_clients; cl++)
		if (clients[cl].sock != -1)
			close(clients[cl].sock);
	
	// Free memory
	free(clients);
	
	puts("Server: shutting down\n");
	return EXIT_SUCCESS;
}
