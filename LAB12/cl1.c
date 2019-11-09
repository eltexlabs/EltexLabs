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
	shutdown(cl_sock, SHD_RD);	// Write only
	
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
		/*result = select(1, &fds, NULL, NULL, &tout);
		printf("%d %d\n", result, FD_ISSET(udp_sock, &fds));
		if (FD_ISSET(udp_sock, &fds))
		{*/
			result = recvfrom(udp_sock, buf, sizeof(buf), 0, (struct sockaddr *) &udp_addr, &sz); //MSG_DONTWAIT
			if (result != FAIL)
			{
				if (!strcmp(buf, "udp1"))
				{
					puts("Got UDP echo from server ...\nSending response ...");
					strcpy(buf, "tcp1");
					send(cl_sock, buf, strlen(buf), 0);
				}
			}
			else
				puts("Nothing on UDP");
		/*}
		else
		{
			puts("Nothing on UDP");
		}*/
		
		sleep(1);
	}
	
	// Close sockets
	close(cl_sock);
	//close(sv_sock);
	
	puts("Client: shutting down ...\n");
	
	
	/*
	fcell_t *pcells;
	short fly_x, fly_y, x, y;
	int size, targets;
	thargs_t * pa;
	int cl_sock;
	struct sockaddr_in saddr_cl, saddr_sv;	// cau, sau;
	char cl_name[108];
	clmsg_result_t msg;
	int temp, try;
	
	// Get args
	pa = (thargs_t *) pv_tharg;
	
	// Init socket
	cl_sock = socket(AF_INET, SOCK_STREAM, 0);		// make socket
	COND_EXIT(cl_sock == FAIL, "socket() error");
	cau.sun_family = AF_INET;						// bind socket
	sprintf(cl_name, "%s%d", CL_ADDR_UN, pa->fdir);
	//strcpy(cau.sun_path, cl_name);	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	unlink(cl_name); // unlink before bind()
	temp = bind(cl_sock, (struct sockaddr *)&cau, sizeof(cau));
	COND_EXIT(temp == FAIL, "bind() error");
	
	// Connect
	sau.sun_family = AF_INET;	
	//strcpy(sau.sun_path, SERV_ADDR_UN);	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	for (try = 0; try < CL_TRYCOUNT; try++)
	{
		sleep(1);	// Period between attempts
		
		if (connect(cl_sock, (struct sockaddr *) &sau, sizeof(sau)) == FAIL)
			printf("Child[%s]: connect attempt #%d failed \n", sflydirs[pa->fdir], try);
		else
			break;
	}
	COND_EXIT(cl_sock == FAIL, "connect() error");
	
	// Calc targets
	printf("Child[%s]: connected, starting ...\n", sflydirs[pa->fdir]);
	fly_x = xflydirs[pa->fdir];
	fly_y = yflydirs[pa->fdir];
	#ifdef DEBUG
	printf("Debug: fdir: %d, name: %s, fly_x: %d, fly_y: %d \n",
		pa->fdir, sflydirs[pa->fdir], fly_x, fly_y);
	#endif
	pcells = pa->pf->pcells;
	size = pa->pf->size;
	targets = 0;
	x = pa->pf->start_x;
	y = pa->pf->start_y;
	while ( (0 <= x && x < size) && (0 <= y && y < size) )
	{
		if (pcells[x + y*size] == FC_TRGT)
			targets++;
		
		x+=fly_x;
		y+=fly_y;
	}
	#ifdef DEBUG
	printf("Child[%s]: found %d target(s)\n", sflydirs[pa->fdir], targets);
	#endif
	
	// Send target count to server
	msg.fdir = pa->fdir;
	msg.targets = targets;
	send(cl_sock, &msg, sizeof(msg), 0);
	
	// Close socket
	close(cl_sock);
	
	// Exit
	pthread_exit(NULL);*/
}
