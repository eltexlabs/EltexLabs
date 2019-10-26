//// 25.10.19
//// LAB #10 - var #11: find targets on the field
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
#include <stdint.h>		// uint32_t and others
#include <stdbool.h>	// BOOL
#include <pthread.h>	// pthread_*** functions
#include <sys/socket.h>			// Sockets
//#include <netinet/in.h>		// Inet
#include <sys/un.h>				// Unix



// Definitions and typedefs //
#define COND_EXIT(COND, EXMSG) { if (COND) { printf("=== Error in %s() ===\n%s\n", __func__, EXMSG); exit(EXIT_FAILURE); } }
#define COND_MSG(COND, MSG) { if (COND) { printf("=== Warning in %s() ===\n%s\n", __func__, MSG); } }
#define COND_TEXIT(COND, EXMSG) { if (COND) { printf("=== Error in %s() ===\n%s\n", __func__, EXMSG); pthread_exit(NULL); } }
#define MIN_FIELD		5	// Min field dimension
#define MAX_FIELD		35	// Max field dimension
#define NUM_THR			8
#define SERV_ADDR_UN	"sock_unix.server"
#define SERV_QSZ		NUM_THR	// Server queue size
#define CL_ADDR_UN		"sock_unix.client"
#define CL_TRYCOUNT		10
#define FAIL			-1	// Typical ret val of unix funcs on error
//#define DEBUG

typedef enum fcell_t	// Field cell
{
	FC_EMPTY = '*',
	FC_TRGT = 'T',
	FC_START = 'S',
} fcell_t;

typedef struct field_t	// Field descriptor
{
	short size;				// Field has size*size cells
	fcell_t * pcells;		// Fileds cells pointer
	short start_x;			// Start coordinates - x
	short start_y;			// Start coordinates - y
} field_t;

typedef enum flydir_t	// Possible fly directions
{
	FD_UL = 0, FD_U, FD_UR, FD_R,	// directions: \ | / -
	FD_BR, FD_B, FD_BL, FD_L,		// directions: \ | / -
} flydir_t;

const char * sflydirs[] = {	// Fly direction names (corresponds to flydir_t)
	"up + left",	"up",	"up + right", "right",
	"bottom + right", "bottom", "bottom + left", "left",
};

const short xflydirs[] = {	// Fly direction x coord increments (corresponds to flydir_t)
	-1, 0, 1, 1,
	1, 0, -1, -1,
};

const short yflydirs[] = {	// Fly direction y coord increments (corresponds to flydir_t)
	-1, -1, -1, 0,
	1, 1, 1, 0,
};

typedef struct clmsg_result_t	// Network message from client with target count
{
	flydir_t fdir;	// Fly direction
	int targets;	// How many targets this thread found
} clmsg_result_t;

typedef struct thargs_t		// Arguments for thread func
{
	field_t * pf;
	flydir_t fdir;
} thargs_t;



// Prototypes //
void field_init(field_t * pf, short size);
void field_deinit(field_t * pf);
void field_print(field_t * pf);
void * thread_main(void * pv_tharg);



void field_init(field_t * pf, short size)
{
	short x, y;
	int sz, targets, t;
	fcell_t * pcells;
	
	// (re)Init randomizer
	srand( time(NULL) );
	
	// Check size
	if (size < MIN_FIELD)
		size = MIN_FIELD;
	if (size > MAX_FIELD)
		size = MAX_FIELD;
	
	// Allocate memory
	sz = size*size*sizeof(fcell_t);	// Field is size*size square array of fcell_t elements
	pf->pcells = malloc(sz);
	COND_EXIT(!pf, "malloc() error");
	
	// Init field with empty cells
	pcells = pf->pcells;
	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
			*pcells++ = FC_EMPTY;
	
	// Place targets
	targets = (size*size) >> 2;	// Max target count is around a quarter of the field size
	for (t = 0; t < targets; t++)
	{
		// Generate coordinates
		x = rand() % size;
		y = rand() % size;
		
		// Place target
		pf->pcells[x + y*size] = FC_TRGT;
	}
	
	// Set size
	pf->size = size;
	
	// Get start pos
	x = rand() % size;
	y = rand() % size;
	pf->start_x = x;
	pf->start_y = y;
	pf->pcells[x + y*size] = FC_START;
}

void field_deinit(field_t * pf)
{
	// Free memory and clean struct
	free(pf->pcells);
	memset(pf, 0x00, sizeof(field_t));
}

void field_print(field_t * pf)
{
	short x, y;
	fcell_t * pcells;
	
	puts("\nField map:");
	
	// Print all field cells
	pcells = pf->pcells;
	for (x = 0; x < pf->size; x++)
	{
		for (y = 0; y < pf->size; y++)
			printf("%c ", *(pcells++) );	// Field cells contain printable characters
		printf("\n");
	}
	puts(" ");
}

void * thread_main(void * pv_tharg)
{
	fcell_t *pcells;
	short fly_x, fly_y, x, y;
	int size, targets;
	thargs_t * pa;
	int cl_sock;
	struct sockaddr_un cau, sau;
	char cl_name[108];
	clmsg_result_t msg;
	int temp, try;
	
	// Get atgs
	pa = (thargs_t *) pv_tharg;
	
	// Init socket
	cl_sock = socket(AF_UNIX, SOCK_STREAM, 0);		// make socket
	COND_TEXIT(cl_sock == FAIL, "socket() error");
	cau.sun_family = AF_UNIX;						// bind socket
	sprintf(cl_name, "%s%d", CL_ADDR_UN, pa->fdir);
	strcpy(cau.sun_path, cl_name);
	unlink(cl_name); // unlink before bind()
	temp = bind(cl_sock, (struct sockaddr *)&cau, sizeof(cau));
	COND_TEXIT(temp == FAIL, "bind() error");
	
	// Connect
	sau.sun_family = AF_UNIX;	
	strcpy(sau.sun_path, SERV_ADDR_UN);
	for (try = 0; try < CL_TRYCOUNT; try++)
	{
		sleep(1);	// Period between attempts
		
		if (connect(cl_sock, (struct sockaddr *) &sau, sizeof(sau)) == FAIL)
			printf("Child[%s]: connect attempt #%d failed \n", sflydirs[pa->fdir], try);
		else
			break;
	}
	COND_TEXIT(cl_sock == FAIL, "connect() error");
	
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
	
	// Close sockets
	close(cl_sock);
	
	// Exit
	pthread_exit(NULL);
}

int main (int argc, char * argv[])
{
	// Parent process
	field_t field;
	int fsize;
	flydir_t fdir;
	int t, temp;
	pthread_t tids[NUM_THR];
	thargs_t thargs[NUM_THR], *tharg;
	int sv_sock, cl_socks[NUM_THR], cl_count, c;
	struct sockaddr_un sau;
	clmsg_result_t cl_msg;
	
	// Get args
	if (argc != 2)
	{
		puts("Ivalid args");
		puts("Usage: [prog] [field_edge_size]");
		printf("[field_edge_size] from %d to %d \n", MIN_FIELD, MAX_FIELD);
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &fsize);
	
	// Create field
	printf("\nParent: creating field ... \n");
	field_init(&field, fsize);
	
	// Print field map
	field_print(&field);
	
	// Init server socket and start listening
	sv_sock = socket(AF_UNIX, SOCK_STREAM, 0);		// make socket
	COND_EXIT(sv_sock == FAIL, "socket() error");
	sau.sun_family = AF_UNIX;						// bind socket
	strcpy(sau.sun_path, SERV_ADDR_UN);
	unlink(SERV_ADDR_UN); // unlink before bind()
	temp = bind(sv_sock, (struct sockaddr *)&sau, sizeof(sau));
	COND_EXIT(temp == FAIL, "bind() error");
	temp = listen(sv_sock, SERV_QSZ);				// listen for incoming connections
	COND_EXIT(temp == FAIL, "listen() error");
	
	// Start threads
	printf("Parent: spawning threads ... \n");
	for (t = 0, fdir = 0, tharg = thargs; t < NUM_THR; t++, fdir++, tharg++)
	{
		// Prepare args
		tharg->fdir = fdir;
		tharg->pf = &field;
		
		// Create thread
		temp = pthread_create(&tids[t], NULL, thread_main, tharg);
		COND_EXIT(temp, "pthread_create() error");
	}
	
	// Accept client connections
	cl_count = 0;
	while (cl_count != NUM_THR)
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
	for (t = 0; t < NUM_THR; t++)
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
