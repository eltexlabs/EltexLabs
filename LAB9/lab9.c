/*
=====================================================================
Copyright (c) 2017-2018, Alexey Leushin
All rights reserved.
Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=====================================================================
*/

//// 18.10.19
//// LAB #9 - var #8: calc mean value of arrays
////



// Includes
#include <stdio.h>		// printf, puts, scanf, gets
//#include <string.h>	// strlen, strcpy
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// pid_t
#include <wait.h>		// waitpid
#include <unistd.h>		// sleep, fork, getpid
#include <time.h>		// time
#include <limits.h>		// INT_MAX
#include <float.h>		// FLT_MAX
#include <sys/ipc.h>	// IPC_* stuff
#include <sys/shm.h>	// shm*() funcs for shared memory
#include <sys/sem.h>	// sem*() funcs for semaphores

// Definitions
#define BUF_LEN	300

#define PM_OTH	0007	// Others
#define PM_GRP	0070	// Group
#define PM_USR	0700	// Owner
#define PM_X	0111	// eXec perm
#define PM_W	0222	// Write perm
#define PM_R	0444	// Read perm

#define S_KEY			IPC_PRIVATE
#define S_FLAGS			PM_R | PM_W | IPC_CREAT

#define FAIL			-1			// fail value of shm*** and sem*** funcs
#define FAIL_PTR		(void *)-1	// fail value of shmat() with type (void*) to fix warning

#define PID_FAIL		-1
#define PID_CHILD		0

#define CHILD_DELAY		1			// Child process delay to illustrate semaphore workflow
#define HOST_UDELAY		100000		// Host process delay
#define MIN_ARRAYS		1
#define MIN_ELEMS		1

struct output_str
{
	int id;
	float mean;
};
typedef struct output_str output_t;

// Semaphore struct #1
union semun_uni {
	int val;				/* value for SETVAL */
	struct semid_ds *buf;	/* buf for IPC_STAT, IPC_SET */
	unsigned short *array;	/* arr for GETALL, SETALL */
	struct seminfo *__buf;	/* buf for IPC_INFO */
};
typedef union semun_uni semun_t;

// Semaphore struct 2
/*struct sembuf_str {
	short sem_num;	// semaphore number ( same as in semctl() )
	short sem_op;	// semaphore operation ( <0 - lock,  >0 - free, 
						//	0 - wait until sem val reaches 0 )
	short sem_flg;	// operation flags ( IPC_NOWAIT )
};
typedef struct sembuf_str sembuf_t;*/
typedef struct sembuf sembuf_t;

// Globals
sembuf_t sem_lock	= {0, -1, 0};	// Predefined struct for locking semaphore #0 by 1
sembuf_t sem_unlock	= {0,  1, 0};	// Predefined struct for unlocking semaphore #0 by 1

// Prototypes
void get_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[], int * p_out_shm_id, int * p_out_sem_id);
void print_in_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[]);
void print_out_array(int * p_arrc, int * p_out_shm_id, int * p_out_sem_id);
void rem_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[], int * p_out_shm_id, int * p_out_sem_id);
int child_main(int pid, int arrc, int in_shm_id, int in_shm_ct, int out_shm_id, int out_sem_id);



void get_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[], int * p_out_shm_id, int * p_out_sem_id)
{
	char buf[BUF_LEN];	// Text buffer
	int a, count, size, v;
	float * p_vals;
	output_t * p_out;
	semun_t semu;
	
	/* === Setup input arrays === */
	puts("\n\nGetting arrays ...");
	
	// Get number of arrays
	printf("Array count: ");
	fgets(buf, sizeof(buf), stdin);
	sscanf(buf, "%d", p_arrc);
	if ((*p_arrc) < MIN_ARRAYS)
		(*p_arrc) = MIN_ARRAYS;
	
	// Allocate memory for IDs and counts
	(*p_shm_ids) = malloc( (*p_arrc) * sizeof(int*) );	// IDs
	(*p_shm_cts) = malloc( (*p_arrc) * sizeof(int*) );	// Element counts
	if ( !(*p_shm_ids) || !(*p_shm_cts) )
	{
		puts("malloc() error");
		exit(EXIT_FAILURE);
	}
	
	// Get all arrays
	for (a = 0; a < (*p_arrc); a++)
	{
		printf("[Array #%d] elements count: ", a+1);
		fgets(buf, sizeof(buf), stdin);
		sscanf(buf, "%d", &count);
		if (count < MIN_ELEMS)
			count = MIN_ELEMS;
		(*p_shm_cts)[a] = count;
		size = sizeof(float) * count;
		
		// Get shared memory for this array
		(*p_shm_ids)[a] = shmget(S_KEY, size, S_FLAGS);
		if ((*p_shm_ids)[a] == FAIL)
		{
			puts("shmget() error");
			exit(EXIT_FAILURE);
		}
		printf("[Array #%d] shmem ID is: %d \n", a+1, (*p_shm_ids)[a]);
		
		// Attach to current array's mem
		p_vals = shmat( (*p_shm_ids)[a], NULL, 0 );
		if (p_vals == FAIL_PTR)
		{
			puts("shmat() error");
			exit(EXIT_FAILURE);
		}
		
		// Get all elements for this array
		for (v = 0; v < count; v++)
		{
			printf("[Array #%d] element #%d value: ", a+1, v+1);
			fgets(buf, sizeof(buf), stdin);
			sscanf(buf, "%f", (p_vals + v));
		}
		
		// Detach from current array's mem
		if (shmdt( p_vals ) == FAIL)
		{
			puts("shmdt() error");
			exit(EXIT_FAILURE);
		}
	}
	
	
	/* === Setup output array and its semaphore === */
	puts("\n\nSetting up output array ...");
	
	// Get shared memory for output array
	size = (*p_arrc) * sizeof(output_t);
	(*p_out_shm_id) = shmget(S_KEY, size, S_FLAGS);
	if ((*p_out_shm_id) == FAIL)
	{
		puts("shmget() error");
		exit(EXIT_FAILURE);
	}
	
	// Attach to output array mem
	p_out = shmat( (*p_out_shm_id), NULL, 0 );
	if (p_out == FAIL_PTR)
	{
		puts("shmat() error");
		exit(EXIT_FAILURE);
	}
	
	// Reset values for all elements in this array
	for (v = 0; v < count; v++)
	{
		p_out[v].id = INT_MAX;
		p_out[v].mean = FLT_MAX;
	}
	
	// Detach from output array mem
	if (shmdt( p_out ) == FAIL)
	{
		puts("shmdt() error");
		exit(EXIT_FAILURE);
	}
	
	// Get semaphore for output array
	(*p_out_sem_id) = semget(S_KEY, 1, S_FLAGS);
	if ((*p_out_sem_id) == FAIL)
	{
		puts("semget() error");
		exit(EXIT_FAILURE);
	}
	
	// Set semaphore value
	semu.val = 1;	// Only one child can use output array
	if (semctl((*p_out_sem_id), 0, SETVAL, semu) == FAIL)
	{
		puts("semctl() error");
		exit(EXIT_FAILURE);
	}
}

void print_in_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[])
{
	int a, v;
	float * p_vals;
	
	puts("\n\nPrinting arrays ...");
	
	// Print all arrays
	for (a = 0; a < (*p_arrc); a++)
	{
		// Attach to current array's mem
		p_vals = shmat( (*p_shm_ids)[a], NULL, 0 );
		if (p_vals == FAIL_PTR)
		{
			puts("shmat() error");
			exit(EXIT_FAILURE);
		}
		
		// Print element values
		printf("[Array #%d, ID=%d] values: ", a+1, (*p_shm_ids)[a]);
		for (v = 0; v < (*p_shm_cts)[a]; v++)
			printf("{%g} ", p_vals[v]);
		puts("");
		
		// Detach from current array's mem
		if (shmdt( p_vals ) == FAIL)
		{
			puts("shmdt() error");
			exit(EXIT_FAILURE);
		}
	}
}

void print_out_array(int * p_arrc, int * p_out_shm_id, int * p_out_sem_id)
{
	int a;
	output_t * p_out;
	
	puts("\n\nPrinting results ...");
	
	// Attach to current array's mem
	p_out = shmat( (*p_out_shm_id), NULL, 0 );
	if (p_out == FAIL_PTR)
	{
		puts("shmat() error");
		exit(EXIT_FAILURE);
	}
	
	// Print result for each array
	for (a = 0; a < (*p_arrc); a++)
		printf("[Array ID=%d] Mean is %g \n", p_out[a].id, p_out[a].mean);
	
	// Detach from current array's mem
	if (shmdt( p_out ) == FAIL)
	{
		puts("shmdt() error");
		exit(EXIT_FAILURE);
	}
}

void rem_arrays(int * p_arrc, int * p_shm_ids[], int * p_shm_cts[], int * p_out_shm_id, int * p_out_sem_id)
{
	int a;
	
	puts("\n\nCleaning up memory ...");
	
	
	/* === Clean up input arrays === */
	
	// Free shared memory
	for (a = 0; a < (*p_arrc); a++)
	{
		printf("[Array #%d, ID=%d] cleaning up ...\n", a+1, (*p_shm_ids)[a]);
		shmctl( (*p_shm_ids)[a], IPC_RMID, NULL );
	}
	
	// Free normal memory
	free(*p_shm_ids);
	free(*p_shm_cts);
	
	
	/* === Clean up output array and its semaphore === */
	
	puts("Cleaning up output array ...");
	
	// Clean shared memory
	shmctl( (*p_out_shm_id), IPC_RMID, NULL );
	
	// Remove semaphore
	semctl((*p_out_sem_id), 0, IPC_RMID);
}


int child_main(int pid, int arrc, int in_shm_id, int in_shm_ct, int out_shm_id, int out_sem_id)
{
	// Child process //
	float sum, mean;
	int c;
	float * p_in;
	output_t * p_out;
	int res;
	
	// Reinit randomizer for each child
	//srand( time(NULL)+pid );
	
	printf("[Child(%d): started]\n", pid);
	
	// Attach to input memory
	p_in = shmat( in_shm_id, NULL, SHM_RDONLY );
	if (p_in == FAIL_PTR)
	{
		printf("[Child(%d): shmat() error]\n", pid);
		return EXIT_FAILURE;
	}
	
	// Calculate mean
	for (c = 0, sum = 0.0f; c < in_shm_ct; c++)
		sum += p_in[c];
	mean = sum / (float) in_shm_ct;
	
	// Detach from input memory
	if (shmdt( p_in ) == FAIL)
	{
		printf("[Child(%d): shmdt() error]\n", pid);
		return EXIT_FAILURE;
	}
	
	// Wait until output memory is free and attach
	printf("[Child(%d): waiting for semaphore]\n", pid);
	if (semop(out_sem_id, &sem_lock, 1) == FAIL)
	{
		printf("[Child(%d): semop() error]\n", pid);
		return EXIT_FAILURE;
	}
	p_out = shmat( out_shm_id, NULL, 0 );
	if (p_out == FAIL_PTR)
	{
		printf("[Child(%d): shmat() error]\n", pid);
		return EXIT_FAILURE;
	}
	
	// Write resut
	for (res = 0; res < arrc; res++)
	{
		if (p_out[res].id == INT_MAX && p_out[res].mean == FLT_MAX)
		{
			p_out[res].id = in_shm_id;
			p_out[res].mean = mean;
			break;
		}
	}
	
	// Wait
	sleep(CHILD_DELAY);
	
	// Detach from output memory and unlock
	if (shmdt( p_out ) == FAIL)
	{
		printf("[Child(%d): shmdt() error]\n", pid);
		return EXIT_FAILURE;
	}
	if (semop(out_sem_id, &sem_unlock, 1) == FAIL)
	{
		printf("[Child(%d): semop() error]\n", pid);
		return EXIT_FAILURE;
	}
	
	// Return
	return EXIT_SUCCESS;
}


int main (int argc, char * argv[])
{
	// Parent process //
	int arrc;			// Number of arrays
	int * shm_ids;		// Shared memory IDs
	int * shm_cts;		// Shared memory element counts
	int out_shm_id;		// Shared memory ID for output array
	int out_sem_id;		// Semaphore ID for output array
	pid_t *pids, wait;	// PIDs
	int status, exval;
	int a, c;			// Counters
	
	// Init randomizer
	//srand( time(NULL) );
	
	// Setup input and output arrays, fill input array
	get_arrays(&arrc, &shm_ids, &shm_cts, &out_shm_id, &out_sem_id);
	
	// Show contents of input arrays
	print_in_arrays(&arrc, &shm_ids, &shm_cts);
	
	// Alloc memory for pids
	pids = malloc( sizeof(pid_t) * arrc );
	
	// Spawn child processes
	puts("\n\nSpawning child processes ...");
	for (a = 0; a < arrc; a++)
	{
		pids[a] = fork();	// Spawn child process
		if (pids[a] == PID_FAIL)
		{
			// Fail
			puts("fork() error");
			exit(EXIT_FAILURE);
		}
		else if (pids[a] == PID_CHILD)
		{
			// Child
			exit( child_main( getpid(), arrc, shm_ids[a], shm_cts[a], out_shm_id, out_sem_id ) );
		}
		// Parent
	}
	
	// Wait for children
	puts("Waiting for children ...");
	a = 0;
	while (a < arrc)
	{
		// Wait for the next period
		usleep(HOST_UDELAY);
		
		// Check all children
		for (c = 0; c < arrc; c++)
		{
			// Check if child has been processed already
			if (pids[c] == 0)
				continue;
			
			// Process child
			wait = waitpid(pids[c], &status, WCONTINUED | WUNTRACED | WNOHANG );
			
			if (wait == PID_FAIL)
			{
				// Error
				puts("waitpid() error");
				exit(EXIT_FAILURE);
			}
			
			if ( wait == pids[c] && WIFEXITED(status) )
			{
				// Child exited
				exval = WEXITSTATUS(status);
				printf("Child(%d) exited with code %d\n", pids[c], exval);
				a++;
				pids[c] = 0; // Mark child as processed
			}
		}
	}
	
	// Print results
	print_out_array(&arrc, &out_shm_id, &out_sem_id);
	
	// Free memory
	free(pids);
	rem_arrays(&arrc, &shm_ids, &shm_cts, &out_shm_id, &out_sem_id);
	
	// Exit
	puts("");
	return EXIT_SUCCESS;
}
