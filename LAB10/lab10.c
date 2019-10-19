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

//// 20.10.19
//// LAB #10 - var #10: calc CRC for files
////



// Includes
#include <stdio.h>		// printf, puts, scanf, gets
//#include <string.h>	// strlen, strcpy
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// pid_t
#include <wait.h>		// waitpid
#include <unistd.h>		// sleep, fork, getpid
//#include <time.h>		// time
#include <stdint.h>		// uint32_t and others
#include <pthread.h>	// pthread_*** functions

// Definitions
#define COND_EXIT(COND, EXMSG) { if (COND) { puts(EXMSG); exit(EXIT_FAILURE); } }

#define TH_DELAY 1	// Artificial delay for child threads

struct tharg_struct
{
	char * filename;
	int num;
	uint32_t * crcs;
	pthread_mutex_t * pcrc_mutex;
};
typedef struct tharg_struct tharg_t;

// Prototypes
void * thread_main(void * pv_tharg);


void * thread_main(void * pv_tharg)
{
	/* Child thread */
	tharg_t * p_tharg;
	int crc;
	
	p_tharg = (tharg_t *) pv_tharg; // Cast void pointer to proper type
	printf("Child thread: working on file \"%s\" ... \n", p_tharg->filename);
	
	// Calculate CRC
	crc = 0xF00D;
	
	// Lock shared CRC table
	printf("Child thread: waiting for mutex ... \n");
	pthread_mutex_lock(p_tharg->pcrc_mutex);
	
	// Write result to CRC table
	(p_tharg->crcs)[p_tharg->num] = crc;
	sleep(TH_DELAY);
	
	// Unlock shared CRC table
	pthread_mutex_unlock(p_tharg->pcrc_mutex);
	
	printf("Child thread: finifshed working on file \"%s\" \n", p_tharg->filename);
	
	pthread_exit(NULL);
}


int main (int argc, char * argv[])
{
	/* Parent process */
	pthread_t *tids;
	tharg_t *thargs, *tharg;
	uint32_t *crcs;
	pthread_mutex_t crc_mutex;
	int t, temp;
	
	// Check arguments
	if (argc < 2)
	{
		puts("Use: [prog] [file1] [file2] ...");
		exit(EXIT_SUCCESS);
	}
	tids = calloc( sizeof(pthread_t) * argc, 1 );	// Alloc memory for thread IDs
	thargs = calloc( sizeof(tharg_t) * argc, 1 );	// Alloc memory for thread args
	crcs = calloc( sizeof(uint32_t) * argc, 1 );	// Alloc memory for CRCs
	
	// Prepare mutex
	pthread_mutex_init(&crc_mutex, NULL);
	
	// Start threads
	printf("Parent: spawning threads ... \n");
	for (t = 0, tharg = thargs; t < argc; t++, tharg++)
	{
		// Prepare args
		tharg->filename = argv[t];
		tharg->num = t;
		tharg->crcs = crcs;
		tharg->pcrc_mutex = &crc_mutex;
		
		// Create thread
		temp = pthread_create(&tids[t], NULL, thread_main, tharg);
		COND_EXIT(temp, "pthread_create() error");
	}
	
	// Wait for all threads
	printf("Parent: waiting for child threads ... \n");
	for (t = 0; t < argc; t++)
	{
		pthread_join(tids[t], NULL); 
	}
	
	// Check results
	printf("Parent: printing results ... \n");
	for (t = 0; t < argc; t++)
	{
		printf("File \"%s\" CRC is: 0x%X\n", argv[t], crcs[t]);
	}
	
	// Free memory
	free(tids);
	free(thargs);
	free(crcs);
	pthread_mutex_destroy(&crc_mutex);
	
	// Exit
	return 0;
}
