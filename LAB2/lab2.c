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

//// 27.09.19
//// LAB #2 
////

// Includes
#include <stdio.h>	// printf, puts, scanf, gets
#include <string.h>	// strlen, strcpy
#include <stdlib.h>	// exit, malloc, calloc, free

// Defines
#define FALSE		0
#define TRUE		1
#define BOOL		int
#define IN_BUF_LEN	100

// Prototypes
void GetStrings(int * pStrCnt, char *** pStrTable);
void DelStrings(int * pStrCnt, char *** pStrTable);
void PrintStrings(int * pStrCnt, char *** pStrTable);
int SortStrings(int * pStrCnt, char *** pStrTable);
int CountWords(char * Str);
void FixNewline(char * FixMe, int Sz);

// char *   - string
// char **  - string table
// char *** - pointer to string table


void FixNewline(char * FixMe, int Sz)
{
	int c;
	
	// Replace first '\n' with '\0'
	for (c = 0; c < Sz && FixMe[c] != '\0'; c++)
		if (FixMe[c] == '\n')
		{
			FixMe[c] = '\0';
			break;
		}
}

void GetStrings(int * pStrCnt, char *** pStrTable)
{
	int s, len;
	char buf[IN_BUF_LEN];
	
	// Get string count
	puts("[Filling string table]\nType number of strings:");
	//scanf("%d", pStrCnt);
	//gets(buf);
	fgets(buf, sizeof(buf), stdin);
	sscanf(buf, "%d", pStrCnt);
	
	// 1 or 2 strings - nope
	if ((*pStrCnt) < 2)
		exit(0);
	
	// Alloc string table
	*pStrTable = (char **) malloc(sizeof(char **) * (*pStrCnt));
	
	// Fill string table
	for (s = 0; s < *pStrCnt; s++)
	{
		printf("Type string #%i: \n", s+1);
		//scanf("%s", buf);
		//gets(buf);
		fgets(buf, sizeof(buf), stdin);
		FixNewline(buf, sizeof(buf));
		len = strlen(buf);
		(*pStrTable)[s] = (char *) calloc(len + 1, 1);
		strcpy((*pStrTable)[s], buf);
	}
}

void DelStrings(int * pStrCnt, char *** pStrTable)
{
	int s;
	
	// Delete strings
	puts("[Freeing memory]");
	for (s = 0; s < *pStrCnt; s++)
	{
		if ((*pStrTable)[s])
			free((*pStrTable)[s]);
	}
	
	// Delete string table
	if (*pStrTable)
		free(*pStrTable);
}

void PrintStrings(int * pStrCnt, char *** pStrTable)
{
	int s;
	
	if (*pStrTable)
	{
		// Print strings
		puts("List of strings:");
		for (s = 0; s < *pStrCnt; s++)
		{
			if ((*pStrTable)[s])
				printf("[#%d]\t%s\n", s+1, (*pStrTable)[s]);
		}
	}
}

int CountWords(char * Str)
{
	int NumWords, c;
	
	if (Str[0] == '\0')
	{
		// Empty string - no words
		NumWords = 0;
	}
	else
	{
		// Non-empty string - at least one word
		NumWords = 1;
		c = 0;
		while (Str[c])
		{
			// One space - one word (not the best solution, but ok)
			if (Str[c] == ' ')
				NumWords++;
			
			// Next symbol
			c++;
		}
	}
	
	return NumWords;
}

int SortStrings(int * pStrCnt, char *** pStrTable)
{
	int s;
	int NumSwaps;
	BOOL IsSorted;
	char FirstSymInLastStr;	// Var5 specific
	
	puts("[Sorting strings]");
	
	// qsort() can't return number of swaps, so bubble sort is used
	NumSwaps = 0;
	IsSorted = FALSE;
	while (!IsSorted)
	{
		// Set sorted flag
		IsSorted = TRUE;
		for (s = 0; s < (*pStrCnt)-1; s++)
		{
			if ( CountWords( (*pStrTable)[s] ) > CountWords( (*pStrTable)[s+1] ) )
			{
				// Improper order - swap
				char * Temp = (*pStrTable)[s];
				(*pStrTable)[s] = (*pStrTable)[s+1];
				(*pStrTable)[s+1] = Temp;
				
				// Reset sorted flag
				IsSorted = FALSE;
				
				// Increase num swaps
				NumSwaps++;
			}
		}
		
	}
	
	// Var5 specific
	FirstSymInLastStr = (*pStrTable)[(*pStrCnt)-1][0];
	puts("---Variant #5---");
	printf("Num swaps: %d \nFirst symbol in the last string: %c \n",
			NumSwaps, FirstSymInLastStr);
	puts("----------------");
	
	return NumSwaps;
}

int main ()
{
	int StrCnt;			// String count in string table
	char ** StrTable;	// String table
	
	
	puts("[main-start]");
	
	// Get strings
	GetStrings(&StrCnt, &StrTable);
	
	// Print strings
	PrintStrings(&StrCnt, &StrTable);
	
	// Sort strings
	SortStrings(&StrCnt, &StrTable);
	
	// Print strings
	PrintStrings(&StrCnt, &StrTable);
	
	// Free memory
	DelStrings(&StrCnt, &StrTable);
	
	// Exit
	puts("[main-end]");
	return 0;
}
