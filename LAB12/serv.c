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
#include <time.h>		// time
#include <pthread.h>	// pthread_*** functions
#include <sys/socket.h>			// Sockets
#include <netinet/in.h>			// Inet
//#include <sys/un.h>			// Unix

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
	int sv_sock, *cl_socks, udp_sock;
	struct sockaddr_in sv_addr, cl_addr, udp_addr;
	int cl, p;	// Counters
	int result, optarg;
	BOOL quit;
	
	puts("Server: starting up ...\n");
	
	// Get args
	if (argc != 4)
	{
		puts("Ivalid args");
		puts("Server usage: [prog] [tcp_port] [udp_port] [num_clients] ");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &tcp_port);
	sscanf(argv[2], "%d", &udp_port);
	/*if (tcp_port < 0 || tcp_port > 0xFFFF || udp_port < 0 || udp_port > 0xFFFF)
	{
		puts("Invalid port number!")
		return EXIT_FAILURE;
	}*/
	sscanf(argv[3], "%d", &max_clients);
	if (max_clients < 1)
		max_clients = 1;
	
	// Alloc mem
	cl_socks = calloc( max_clients, sizeof(*cl_socks) );
	
	// Prepare TCP client sockets
	num_clients = 0;
	
	// Prepare UDP server socket
	puts("Server: opening UDP socket ...");
	PrepInetSock(NULL, udp_port, udp_sock, udp_addr, true);
	/*udp_sock = socket(AF_INET, SOCK_DGRAM, 0);		// Make UDP socket
	COND_EXIT(udp_sock == FAIL, "socket() error");
	optarg = 1;
	result = setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optarg, sizeof(optarg));	// Multiple local UDP clients
	COND_EXIT(result == FAIL, "setsockopt() error");
	result = setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optarg, sizeof(optarg));	// Enable UDP broadcast
	COND_EXIT(result == FAIL, "setsockopt() error");
	//result = setsockopt(udp_sock, SOL_SOCKET, SO_RCVLOWAT, (char *)&msg_len, sizeof(msg_len));
	memset(&udp_addr, 0x00, sizeof(udp_addr));		// Addr: clear struct
    udp_addr.sin_family = AF_INET;					// Addr: Internet
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// Addr: Any address for incoming connections
    udp_addr.sin_port = htons(udp_port);			// Addr: Port number for incoming connections
	result = bind(udp_sock, (struct sockaddr *) &udp_addr, sizeof(udp_addr));	// Bind UDP socket
	COND_EXIT(result == FAIL, "bind() error");
	printf("Server: UDP socket opened (port %d)\n", udp_port);*/
	
	// Prepare TCP server socket
	puts("Server: opening TCP socket ...");
	PrepInetSock(NULL, tcp_port, sv_sock, sv_addr, false);
	/*sv_sock = socket(AF_INET, SOCK_STREAM, 0);		// Make TCP socket
	COND_EXIT(sv_sock == FAIL, "socket() error");
	memset(&sv_addr, 0x00, sizeof(sv_addr));		// Addr: clear struct
    sv_addr.sin_family = AF_INET;					// Addr: Internet
    sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// Addr: Any address for incoming connections
    sv_addr.sin_port = htons(tcp_port);				// Addr: Port number for incoming connections
	result = bind(sv_sock, (struct sockaddr *) &sv_addr, sizeof(sv_addr));	// Bind TCP socket
	COND_EXIT(result == FAIL, "bind() error");
	printf("Server: TCP socket opened (port %d)\n", tcp_port);*/
	result = listen(sv_sock, SERV_QSZ);				// Start listening TCP socket
	COND_EXIT(result == FAIL, "listen() error");
	
	// Main loop
	while (!UserQuit())
	{
		// Check connections
		// Check message queue
			// not full: Recieve incoming data (select(), peek())
				// still not full: Send UDP for cl1
			// full: send messages
				// send UDP for cl2
		
		// Send UDP request to client
		 if (sendto(udp_sock, "udp1", sizeof("udp1"), 0, (struct sockaddr *) &udp_addr,
		sizeof(udp_addr)) != sizeof("udp1"))
		{
			puts("sendto() error");
			exit(1);
		}
		
		sleep(1);
	}
	
	// Close sockets
	close(udp_sock);
	close(sv_sock);
	for (cl = 0; cl < num_clients; cl++)
		close(cl_socks[cl]);
	
	// Free memory
	free(cl_socks);
	
	puts("Server: shutting down\n");
	return EXIT_SUCCESS;
	
	/*//field_t field;
	//int fsize;
	//flydir_t fdir;
	int t, temp;
	//pthread_t tids[NUM_THR];
	//thargs_t thargs[NUM_THR], *tharg;
	int num_clients;
	int sv_sock, cl_socks[NUM_THR], cl_count, c;
	struct sockaddr_in saddr_sv; //sau;
	clmsg_result_t cl_msg;
	int th_count, x, y;
	
	// Get args
	if (argc != 2)
	{
		puts("Ivalid args");
		puts("Server usage: [prog] [num_clients]");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &num_clients);
	
	// Init server socket and start listening
	sv_sock = socket(AF_INET, SOCK_STREAM, 0);		// make socket
	COND_EXIT(sv_sock == FAIL, "socket() error");
	sau.sun_family = AF_INET;						// bind socket
	//strcpy(sau.sun_path, SERV_ADDR_UN);	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	unlink(SERV_ADDR_UN); // unlink before bind()
	temp = bind(sv_sock, (struct sockaddr *)&sau, sizeof(sau));
	COND_EXIT(temp == FAIL, "bind() error");
	temp = listen(sv_sock, SERV_QSZ);				// listen for incoming connections
	COND_EXIT(temp == FAIL, "listen() error");
	
	// Start threads
	printf("Parent: spawning threads ... \n");
	th_count = 0;
	for (t = 0, fdir = 0, tharg = thargs; t < NUM_THR; t++, fdir++, tharg++)
	{
		// Check direction (filter out of bounds threads)
		x = field.start_x + xflydirs[fdir];
		y = field.start_y + yflydirs[fdir];
		if ( (0 > x || x >= field.size) || (0 > y || y >= field.size) )
		{
			#ifdef DEBUG
			printf("Parent: skipping direction [%s] \n", sflydirs[fdir]);
			#endif
			
			continue;
		}
		
		// Prepare args
		tharg->fdir = fdir;
		tharg->pf = &field;
		
		// Create thread
		temp = pthread_create(&tids[th_count], NULL, thread_main, tharg);
		COND_EXIT(temp, "pthread_create() error");
		th_count++;
	}
	
	// Accept client connections
	cl_count = 0;
	while (cl_count != th_count)
	{
		cl_socks[cl_count] = accept(sv_sock, (struct sockaddr *) NULL, 0);	// (server_sock, client_addr, client_addr_sz)
		COND_MSG(cl_socks[cl_count] == FAIL, "accept() error");
		cl_count++;
	}
	
	// Get results
	for (c = 0; c < cl_count; c++)
	{
		if (cl_socks[c] != FAIL)
		{
			//temp = setsockopt(cl_sock, SOL_SOCKET, SO_RCVLOWAT, (char *)&msg_len, sizeof(msg_len));
			temp = recv(cl_socks[c], &cl_msg, sizeof(cl_msg), 0);
			if (temp == sizeof(cl_msg))
				printf("Parent: got message from [%s]\n \ttarget count is %d \n",
					sflydirs[cl_msg.fdir], cl_msg.targets);
			else
				puts("Parent: oops, recv() got half-backed message, ToDo: fix with setsockopt()");
		}
	}
	
	// Wait for all threads to exit
	printf("Parent: waiting for child threads to exit ... \n");
	for (t = 0; t < th_count; t++)
		pthread_join(tids[t], NULL);
	
	// Close sockets
	close(sv_sock);
	for (c = 0; c < cl_count; c++)
		if (cl_socks[c] != FAIL)
			close(cl_socks[c]);
	
	// Free memory
	puts("Parent: cleaning memory ...");
	field_deinit(&field);
	
	puts("Parent: done\n");
	return 0;*/
}
