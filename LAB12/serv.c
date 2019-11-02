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


// Prototypes //


// Main code //

int main (int argc, char * argv[])
{
	// Parent process
	//field_t field;
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
	return 0;
}
