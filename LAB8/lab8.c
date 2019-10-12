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

//// 11.10.19
//// LAB #8 - var #7: Team DM
////



// Includes
#include <stdio.h>		// printf, puts, scanf, gets
//#include <string.h>	// strlen, strcpy
#include <stdlib.h>		// exit, malloc, calloc, free, qsort, rand
#include <sys/types.h>	// pid_t
#include <wait.h>		// waitpid
#include <unistd.h>		// usleep, sleep, fork, getpid, pipe
#include <time.h>		// time
#include <sys/msg.h>	// msgget, msgsnd, msgctl


// Definitions
//#define DEBUG

#define TEAM_MAXTEAMS	2000	// Maximum number of teams
#define TEAM_START_SZ	10		// Number of team members at start
#define TEAM_MAXGROW	5		// How much team can grow per activity period
#define TEAM_MINKILLS	2		// Minimum amplitude of damage than can be caused by one team to another
#define TEAM_WAIT		100000	// Team activity period: 0.1 sec

#define HOST_WAIT		100000	// Host activity period: 0.1 sec

#define MSG_PFLAGS	0600 | IPC_CREAT | IPC_EXCL		// Message flags
#define MSG_TYPE	1								// Message type
#define MSG_KEY		IPC_PRIVATE						// Message key

#define PID_FAIL		-1
#define PID_CHILD		0
#define PID_SPECIAL		-300

typedef int BOOL;
#define TRUE	1
#define FALSE 	0

struct STRMsgData	// Message data
{
	int numkills;	// Nember of kills requested
	int pid;		// pid of request origin
};
typedef struct STRMsgData SMsgData;

struct STRMsg	// Sys-V message
{
	long type;
	SMsgData data;
};
typedef struct STRMsg SMsg;


// Prototypes
int child_main(int pid, int my_msqid, int * msqids, int teams);


int child_main(int pid, int my_msqid, int * msqids, int teams)
{
	//// Child process (Team) ////
	int members, newmembers, t, temp, totaldmg;
	BOOL winner, dead;
	SMsg msgbody;
	
	// Reinit randomizer for each child
	srand( time(NULL)+pid );
	
	// Delay to wait for all teams to spawn
	sleep(1);
	
	// Start
	members = TEAM_START_SZ;
	printf("[Team(%i): started with %d members]\n", pid, members);
	
	// Loop until dead or victory
	winner = dead = FALSE;
	while (!dead && !winner)
	{
		// Collect damage from other teams / get win notification from host
		totaldmg = 0;
		while ( -1 != (temp = msgrcv(my_msqid, &msgbody, sizeof(msgbody.data), 0, IPC_NOWAIT)) )
		{
			if (msgbody.data.pid == PID_SPECIAL && msgbody.data.numkills == msgbody.data.pid)
			{
				// Win notification from host
				printf("[Team(%d): got win notification from host]\n", pid);
				winner = TRUE;
				break;
			}
			
			totaldmg += msgbody.data.numkills;
			members -= msgbody.data.numkills;
			#ifdef DEBUG
			printf("[Team(%d): msgrcv=%d]\n", pid, temp);
			printf("[Team(%d): %d members killed by team(%d), members count is %d]\n", 
				pid, msgbody.data.numkills, msgbody.data.pid, members);
			#endif
			if (members < 1)
			{
				// Dead
				#ifdef DEBUG
				printf("[Team(%d): closing msg queue\n", pid);
				#endif
				dead = TRUE;
				for (t = 0; t < teams; t++)
				{
					if (msqids[t] == my_msqid)	// Find self ID in IDs
					{
						msgctl(my_msqid, IPC_RMID, 0);	// Close message queue
						//msqids[t] = MSG_ID_DEAD;		// Mark self as dead
						break;
					}
				}
				break;
			}
		}
		
		if (!dead && !winner)
		{
			// Spawn new members
			newmembers = rand() % (TEAM_MAXGROW+1);
			members += newmembers;
			#ifdef DEBUG
			printf("[Team(%d): got %d new members, members count is %d]\n", pid, newmembers, members);
			#endif
			
			// Give damage to other teams
			#ifdef DEBUG
			printf("[Team(%d): damaging other teams]\n", pid);
			#endif
			for (t = 0; t < teams; t++)
			{
				int cur_msqid = msqids[t];
				
				// Don't damage self
				if (cur_msqid == my_msqid)
					continue;
				//if (cur_msqid == MSG_ID_DEAD) { continue; }
				
				// Send damage request
				#ifdef DEBUG
				printf("[Team(%d): sending damage to ID %d]\n", pid, cur_msqid);
				#endif
				msgbody.type = MSG_TYPE;
				msgbody.data.pid = pid;
				msgbody.data.numkills = (rand() % (TEAM_MINKILLS + members/10));	// Damage is proportional to number of members
				#ifdef DEBUG
				printf("[Team(%d): send() start]\n", pid);
				#endif
				if ( msgsnd(cur_msqid, &msgbody, sizeof(msgbody.data), IPC_NOWAIT) )
				#ifdef DEBUG
					puts("msgsnd() unsuccessful");
				printf("[Team(%d): send() end]\n", pid);
				#else
					;
				#endif
			}
			
			// Report status
			printf("[Team(%d): dmg=%d,\t income=%d,\t members=\t%d]\n",
				pid, totaldmg, newmembers, members );
			
			// Wait for next cycle
			usleep(TEAM_WAIT);
		}
	}
	
	if (winner)
		printf("[Team(%d): I won, yay]\n", pid);
	if (dead)
		printf("[Team(%d): dead]\n", pid);
	
	if (!winner && !dead)
	{
		printf("[Team(%d): oops, not dead and not winner]\n", pid);
		return EXIT_FAILURE;
	}
	
	// Exit
	printf("[Team(%i): exiting]\n", pid);
	return EXIT_SUCCESS;
}


int main (int argc, char * argv[])
{
	//// Parent process (Host) ////
	pid_t *pids, wait;
	int c, status, exval;
	int teams, t, aliveteams;
	int *msqids;	// Message queue IDs
	SMsg msg;
	
	// Init randomizer
	srand( time(NULL) );
	
	// Get arguments
	if (argc != 2)
	{
		puts("Use: [prog] [num_teams]");
		exit(EXIT_SUCCESS);
	}
	sscanf(argv[1], "%d", &teams);
	if (teams < 2)
		teams = 2;
	if (teams > TEAM_MAXTEAMS)
		teams = TEAM_MAXTEAMS;
	
	// Alloc mem
	pids = malloc( sizeof(pid_t) * teams );	// Alloc memory for pids
	msqids = malloc( sizeof(int) * teams );	// Alloc memory for msg queue IDs
	if (!pids || !msqids)
	{
		puts("malloc() error");
		exit(EXIT_FAILURE);
	}
	
	// Start
	printf("[Host: started, PID=%d]\n", getpid());
	
	// Create message queue for each team
	puts("[Host: making queues]");
	for (t = 0; t < teams; t++)
	{
		msqids[t] = msgget(MSG_KEY, MSG_PFLAGS);
		if (msqids[t] < 0)
		{
			puts("msqids() error");
			exit(EXIT_FAILURE);
		}
	}
	
	// Release the teams
	puts("[Host: releasing teams ...]");
	aliveteams = teams;
	for (t = 0; t < teams; t++)
	{
		pids[t] = fork();			// Spawn child process
		if (pids[t] == PID_FAIL)
		{
			// Fail
			puts("fork() error");
			exit(EXIT_FAILURE);
		}
		else if (pids[t] == PID_CHILD)
		{
			// Child
			exit( child_main( getpid(), msqids[t], msqids, teams ) );
		}
		// Parent
	}
	
	// Watch for teams
	puts("[Host: watching for teams ...]");
	c = 0;
	while (c < teams)
	{
		// Wait for the next cycle
		usleep(HOST_WAIT);
		
		// Check all children
		for (t = 0; t < teams; t++)
		{
			// Check if child has been processed already
			if (pids[t] == 0)
				continue;
			
			// Check if last one left
			if (aliveteams == 1)
			{
				// We found the winner
				printf("[Host: team(%d) won the game]\n", pids[t]);
				// Notify winner
				msg.type = MSG_TYPE;
				msg.data.pid = msg.data.numkills = PID_SPECIAL;
				if ( msgsnd(msqids[t], &msg, sizeof(msg.data), IPC_NOWAIT) ) // Notify winner
					puts("msgsnd() to winner unsuccessful");
			}
			
			// Process child
				// WNOHANG - don't stop if process is running
			wait = waitpid(pids[t], &status, WCONTINUED | WUNTRACED | WNOHANG );
			
			if (wait == PID_FAIL)
			{
				// Error
				printf("waitpid() error for pid %d \n", pids[t]);
				exit(EXIT_FAILURE);
			}
			
				// wait - contains pid of child whose state has changed
				// WIFEXITED - returns non-zero if child exited
			if ( wait == pids[t] && WIFEXITED(status) )
			{
				// Child exited
				exval = WEXITSTATUS(status);
				printf("[Host: team(%d) left the game with status %d]\n", pids[t], exval);
				c++;
				pids[t] = 0;  // Mark child as processed
				aliveteams--; // Reduce number of alive teams (global)
				printf("[Host: %d team(s) remainig]\n", aliveteams);
			}
		}
	}
	puts("[Host: all teams have left the game, exiting ...]");
	
	// Free memory
	free(pids);
	free(msqids);
	
	// Exit
	return 0;
}
