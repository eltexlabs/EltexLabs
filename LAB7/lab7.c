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

//// 10.10.19
//// LAB #7 - var #6: Ballz
////



// Includes
#include <stdio.h>		// printf, puts, scanf, gets
//#include <string.h>	// strlen, strcpy
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// pid_t
#include <wait.h>		// waitpid
#include <unistd.h>		// usleep, sleep, fork, getpid, pipe
#include <time.h>		// time


// Definitions
#define FIELD_X_MIN		-300	// Field x borders
#define FIELD_X_MAX		300		//
#define FIELD_Y_MIN		-300	// Field y borders
#define FIELD_Y_MAX		300		//
#define FIELD_PERIOD	10000	// Field update period: 0.01 sec
//#define DEBUG

#define BALL_MAXBALLS	1000	// Maximum number of balls (prevent crash)
#define BALL_XSTEP_MAX	50		// Max step size a ball can make
#define BALL_YSTEP_MAX	50		//
#define BALL_MV_PERIOD	100000	// Ball move period: 0.1 sec

#define PID_FAIL		-1
#define PID_CHILD		0

struct STRPipe	// Structified pipe descriptors
{
	int rd;
	int wr;
};
typedef struct STRPipe SPipe;

struct STRMsg	// Message with ball coordinates
{
	int x;
	int y;
};
typedef struct STRMsg SMsg;

// Prototypes
int child_main(int pid, SPipe pipe);


int child_main(int pid, SPipe pipe)
{
	//// Child process (Ball) ////
	int posx, posy, mvx, mvy;
	SMsg msg;
	
	// Reinit randomizer for each child
	srand( time(NULL)+pid );
	
	// Start
	posx = posy = 0;
	printf("[Ball(%i): spawned at (0,0)]\n", pid);
	
	// Close read
	close(pipe.rd);
	
	// Move around until host kills this process
	for(;;)
	{
		// Wait for next movement time
		usleep(BALL_MV_PERIOD);
		
		// Randomize move distances (from -maxstep to maxstep)
		mvx = -BALL_XSTEP_MAX + ( rand() % ((BALL_XSTEP_MAX<<1)+1) );
		mvy = -BALL_YSTEP_MAX + ( rand() % ((BALL_YSTEP_MAX<<1)+1) );
		
		// Move
		posx += mvx;
		posy += mvy;
		
		// Debug
		#ifdef DEBUG
		printf("[Ball(%i): moved to (%d,%d) by (%d,%d)]\n", 
			pid, posx, posy, mvx, mvy);
		#endif
		
		// Send coordinates to host
		msg.x = posx;
		msg.y = posy;
		write(pipe.wr, &msg, sizeof(msg));
	}
	
	// Exit (should be unreachable)
	printf("[Ball(%i): exiting]\n", pid);
	return EXIT_SUCCESS;
}


int main (int argc, char * argv[])
{
	//// Parent process (Host) ////
	int balls, b, c, numrd;
	pid_t *pids;
	SPipe * pipes;
	SMsg msg;
	
	// Init randomizer
	srand( time(NULL) );
	
	// Get arguments
	if (argc != 2)
	{
		puts("Use: [prog] [num_balls]");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &balls);
	if (balls <= 0)
		balls = 1;
	if (balls > BALL_MAXBALLS)
		balls = BALL_MAXBALLS;
	
	// Alloc mem
	pids = malloc( sizeof(pid_t) * balls );	 // Alloc memory for pids
	pipes = malloc( sizeof(SPipe) * balls ); // Alloc memory for pipe descriptors
	if (!pids || !pipes)
	{
		puts("malloc() error");
		exit(EXIT_FAILURE);
	}
	
	// Start
	printf("[Host: started, PID=%d]\n", getpid());
	
	// Release the balls on the field
	puts("[Host: spawning balls ...]");
	for (b = 0; b < balls; b++)
	{
		if ( pipe((int *) &pipes[b]) )	// Open pipe
		{
			// Fail
			puts("pipe() error");
			exit(EXIT_FAILURE);
		}
		pids[b] = fork();			// Spawn child process
		if (pids[b] == PID_FAIL)
		{
			// Fail
			puts("fork() error");
			exit(EXIT_FAILURE);
		}
		else if (pids[b] == PID_CHILD)
		{
			// Child
			exit( child_main( getpid(), pipes[b] ) );
		}
		// Parent
		close(pipes[b].wr);	// Close write
	}
	
	// Watch for spawned balls, remove them once they are OOB
	puts("[Host: watching for balls ...]");
	c = 0;
	while (c < balls)
	{
		// read() hangs process anyway
			// Wait for the next check period
			//usleep(FIELD_PERIOD);
		
		// Check all children
		for (b = 0; b < balls; b++)
		{
			// Check if child has been terminated already
			if (pids[b] == 0)
				continue;
			
			// Get coordinates
			numrd = read(pipes[b].rd, &msg, sizeof(msg));
			#ifdef DEBUG
			printf("numrd=%d\n" ,numrd);
			#endif
			
			// Check bounds
			if (numrd)
			{
				if ( msg.x < FIELD_X_MIN || FIELD_X_MAX < msg.x ||
						msg.y < FIELD_Y_MIN || FIELD_Y_MAX < msg.y )
				{
					printf("[Host: ball(%d) is out of bounds coord:(%d,%d)]\n", pids[b], msg.x, msg.y);
					kill(pids[b], SIGKILL);	// Terminate child process
					pids[b] = 0;	// Mark child proc. as termintated
					c+=1;			// Increase number of out of bounds balls
					printf("[Host: %d balls left]\n", balls-c);
				}
			}
			else
				puts("[Host: empty read()]");
			
			// ToDo: check if killed externally?
		}
	}
	puts("[Host: all balls are out of bounds, exiting ...]");
	
	// Free memory
	free(pids);
	free(pipes);
	
	// Exit
	return 0;
}
