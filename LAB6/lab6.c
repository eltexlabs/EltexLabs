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

//// 04.10.19
//// LAB #6 - var #5: Winnie Pooh and bees
////

/* === From: https://linux.die.net/man/2/wait ===
 * 
 * WNOHANG - return immediately if no child has exited.
 * 
 * WUNTRACED - also return if a child has stopped (but not traced via ptrace(2)). 
 * Status for traced children which have stopped is provided even if this option is not specified.
 * 
 * WCONTINUED (since Linux 2.6.10) - also return if a stopped child has been resumed by 
 * delivery of SIGCONT. (For Linux-only options, see below.) If status is not NULL, wait() 
 * and waitpid() store status information in the int to which it points. This integer 
 * can be inspected with the following macros (which take the integer itself as an argument, 
 * not a pointer to it, as is done in wait() and waitpid()!):
 * 
 * 
 * 
 * WIFEXITED(status) - returns true if the child terminated normally, that is, by calling 
 * exit(3) or _exit(2), or by returning from main().
 * 
 * WEXITSTATUS(status) - returns the exit status of the child. This consists of the least 
 * significant 8 bits of the status argument that the child specified in a call to exit(3) 
 * or _exit(2) or as the argument for a return statement in main(). This macro should only 
 * be employed if WIFEXITED returned true.
 * 
 * WIFSIGNALED(status) - returns true if the child process was terminated by a signal.
 * 
 * WTERMSIG(status) - returns the number of the signal that caused the child process 
 * to terminate. This macro should only be employed if WIFSIGNALED returned true.
 * 
 * WCOREDUMP(status) - returns true if the child produced a core dump. This macro 
 * should only be employed if WIFSIGNALED returned true. This macro is not specified 
 * in POSIX.1-2001 and is not available on some UNIX implementations (e.g., AIX, SunOS). 
 * Only use this enclosed in #ifdef WCOREDUMP ... #endif.
 * 
 * WIFSTOPPED(status) - returns true if the child process was stopped by delivery 
 * of a signal; this is only possible if the call was done using WUNTRACED or when 
 * the child is being traced (see ptrace(2)).
 * 
 * WSTOPSIG(status) - returns the number of the signal which caused the child to stop. 
 * This macro should only be employed if WIFSTOPPED returned true.
 * 
 * WIFCONTINUED(status) - (since Linux 2.6.10) returns true if the child process 
 * was resumed by delivery of SIGCONT.
 * */



// Includes
#include <stdio.h>		// printf, puts, scanf, gets
//#include <string.h>	// strlen, strcpy
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// pid_t
#include <wait.h>		// waitpid
#include <unistd.h>		// sleep, fork, getpid
#include <time.h>		// time


// Definitions
#define BEE_MIN_TIME	2
#define BEE_MAX_TIME	7
#define BEE_MIN_HONEY	1
#define BEE_MAX_HONEY	10
#define BEE_WORK_TIME	2

#define PID_FAIL		-1
#define PID_CHILD		0


// Prototypes
int child_main(int pid);


int child_main(int pid)
{
	// Child process (Bee) //
	int flytime, honey;
	
	
	// Reinit randomizer for each child
	srand( time(NULL)+pid );
	
	printf("[Bee(%i): woke up]\n", pid);
	
	// Assess time for travel
	flytime = rand() % (BEE_MAX_TIME+1);
	if (flytime < BEE_MIN_TIME)
		flytime = BEE_MIN_TIME;
	printf("[Bee(%i): on the way, road would take %d seconds]\n", pid, flytime);
	sleep(flytime);
	
	// Get honey
	printf("[Bee(%i): arrived to destination, working]\n", pid);
	sleep( 1 + (rand() % BEE_WORK_TIME) );
	honey = rand() % (BEE_MAX_HONEY+1);
	if (honey < BEE_MIN_HONEY)
		honey = BEE_MIN_HONEY;
	
	// Return home
	printf("[Bee(%i): got %d honey, returning back]\n", pid, honey);
	sleep(flytime);
	
	// Give away honey
	printf("[Bee(%i): back home, giving away %d honey]\n", pid, honey);
	return honey;
}


int main (int argc, char * argv[])
{
	// Parent process (Winnie) //
	int hunger, honey, maxhoney, bees, b, status, c, exval;
	pid_t *pids, wait;
	
	
	// Init randomizer
	srand( time(NULL) );
	
	// Get arguments
	if (argc != 2)
	{
		puts("Use: [prog] [num_bees]");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &bees);
	if (bees <= 0)
		bees = 1;
	pids = malloc( sizeof(pid_t) * bees ); // Alloc memory for pids
	
	// Winnie's hungry
	puts("[Winnie woke up]");
	maxhoney = bees * BEE_MAX_HONEY;
	hunger = (maxhoney >> 2);
	hunger += rand() % (maxhoney >> 1);
	printf("[Winnie needs %d honey to live another day]\n", hunger);
	
	// Release the bees
	puts("[Winnie is releasing bees ...]");
	honey = 0;
	for (b = 0; b < bees; b++)
	{
		pids[b] = fork();	// Spawn child process
		if (pids[b] == PID_FAIL)
		{
			// Fail
			puts("fork() error");
			exit(EXIT_FAILURE);
		}
		else if (pids[b] == PID_CHILD)
		{
			// Child
			exit( child_main( getpid() ) );
		}
		// Parent
	}
	
	// Collect honey
	puts("[Winnie is waiting for bees ...]");
	c = 0;
	while (c < bees)
	{
		// Check period: one sec
		sleep(1);
		
		// Check all children
		for (b = 0; b < bees; b++)
		{
			// Check if child has been processed already
			if (pids[b] == 0)
				continue;
			
			// Process child
				// WNOHANG - don't stop if process is running
			wait = waitpid(pids[b], &status, WCONTINUED | WUNTRACED | WNOHANG );
			
			if (wait == PID_FAIL)
			{
				// Error
				puts("waitpid() error");
				exit(EXIT_FAILURE);
			}
			
				// wait - contains pid of child whose state has changed
				// WIFEXITED - returns non-zero if child exited
			if ( wait == pids[b] && WIFEXITED(status) )
			{
				// Child exited
				exval = WEXITSTATUS(status);
				honey += exval;
				printf("[Winnie got %d honey from Bee(%d)]\n", exval, pids[b]);
				c++;
				pids[b] = 0; // Mark child as processed
			}
		}
	}
	printf("[Winie got total %d honey of %d]\n", honey, hunger);
	
	// Check if there is enough honey
	if (honey < hunger)
		puts("[Winnie starved to death =( ]");
	else
		puts("[Winnie had a good meal =) ]");
	
	// Free memory
	free(pids);
	
	// Exit
	return 0;
}
