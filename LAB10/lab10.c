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
//// LAB #10 - var #10: calculate checksums for files
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
#define TH_DELAY	1			// Artificial delay for child threads
//#define DEBUG


struct tharg_struct
{
	char * filename;
	int num;
	uint32_t * csums;
	pthread_mutex_t * pcsum_mutex;
};
typedef struct tharg_struct tharg_t;

// Prototypes
void * thread_main(void * pv_tharg);


void * thread_main(void * pv_tharg)
{
	/* Child thread */
	tharg_t * p_tharg;
	uint32_t csum;
	FILE * f;
	int c;
	
	p_tharg = (tharg_t *) pv_tharg; // Cast void pointer to proper type
	printf("Child thread: working on file \"%s\" ... \n", p_tharg->filename);
	
	// Calculate checksum
	csum = 0;
	f = fopen(p_tharg->filename, "r");	// Open file
	if (f)
	{
		// Add together all bytes from file
		while ( !feof(f) )
		{
			c = fgetc(f);
			if (c != EOF)	// Prevent -1 going into checksum
			{
				#ifdef DEBUG
				printf("0x%X ", c);
				#endif
				csum += c;
			}
		}
		#ifdef DEBUG
		puts(" ");
		#endif
		
		// Close file
		fclose(f);
	}
	else
	{
		printf("Child thread: can't open file \"%s\" \n", p_tharg->filename);
		csum = 0xFFFFFFFF;
	}
	
	// Lock shared checksum table
	printf("Child thread: waiting for mutex ... \n");
	pthread_mutex_lock(p_tharg->pcsum_mutex);
	
	// Write result to checksum table
	(p_tharg->csums)[p_tharg->num] = csum;
	sleep(TH_DELAY);
	
	// Unlock shared checksum table
	pthread_mutex_unlock(p_tharg->pcsum_mutex);
	
	printf("Child thread: finished working on file \"%s\" \n", p_tharg->filename);
	
	pthread_exit(NULL);
}


int main (int argc, char * argv[])
{
	/* Parent process */
	int nfiles;
	char ** fnames;
	pthread_t *tids;
	tharg_t *thargs, *tharg;
	uint32_t *csums;
	pthread_mutex_t csum_mutex;
	int t, temp;
	
	// Check arguments
	if (argc < 2)
	{
		puts("Use: [prog] [file1] [file2] ...");
		exit(EXIT_SUCCESS);
	}
	nfiles = argc - 1;
	fnames = &argv[1];
	tids = calloc( sizeof(pthread_t) * nfiles, 1 );	// Alloc memory for thread IDs
	thargs = calloc( sizeof(tharg_t) * nfiles, 1 );	// Alloc memory for thread args
	csums = calloc( sizeof(uint32_t) * nfiles, 1 );	// Alloc memory for checksums
	
	// Prepare mutex
	pthread_mutex_init(&csum_mutex, NULL);
	
	// Start threads
	printf("Parent: spawning threads ... \n");
	for (t = 0, tharg = thargs; t < nfiles; t++, tharg++)
	{
		// Prepare args
		tharg->filename = fnames[t];
		tharg->num = t;
		tharg->csums = csums;
		tharg->pcsum_mutex = &csum_mutex;
		
		// Create thread
		temp = pthread_create(&tids[t], NULL, thread_main, tharg);
		COND_EXIT(temp, "pthread_create() error");
	}
	
	// Wait for all threads
	printf("Parent: waiting for child threads ... \n");
	for (t = 0; t < nfiles; t++)
	{
		pthread_join(tids[t], NULL); 
	}
	
	// Check results
	printf("Parent: printing results ... \n");
	for (t = 0; t < nfiles; t++)
	{
		printf("File \"%s\" checksum is: 0x%X\n", fnames[t], csums[t]);
	}
	
	// Free memory
	free(tids);
	free(thargs);
	free(csums);
	pthread_mutex_destroy(&csum_mutex);
	
	// Exit
	return 0;
}
