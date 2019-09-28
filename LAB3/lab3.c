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

//// 28.09.19
//// LAB #3
////

// Includes
#include <stdio.h>	// printf, puts, scanf, gets
#include <string.h>	// strlen, strcpy
#include <stdlib.h>	// exit, malloc, calloc, free, qsort


// Definitions
#define BUF_LEN	80
struct STRPurchase		// Variant #5
{
	char Name[BUF_LEN];
	int Day;
	int Mon;			// Sort by month
	int Year;
	float Cost;
	int Num;
};
typedef struct STRPurchase SPurchase;


// Prototypes
void GetPur(SPurchase *** pPurTab, int * pCount);
void DelPur(SPurchase *** pPurTab, int * pCount);
int  CmpPur(const void * ppvPur1, const void * ppvPur2);
void SortPur(SPurchase ** PurTab, int Count);
void PrintPur(SPurchase ** PurTab, int Count);
void FixNewline(char * FixMe, int Sz);

// struct		- struct (start of struct data)
// struct * 	- struct pointer
// struct ** 	- struct pointer table
// struct ***	- pointer to struct pointer table


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

void GetPur(SPurchase *** pPurTab, int * pCount)
{
	char Buf[BUF_LEN];
	int p;
	unsigned int Day, Mon, Year;
	float Cost;
	unsigned int Num;
	SPurchase * Current;	// Structure that is processed now
	
	// Get stuct count
	puts("[Allocating and filling structures]\nType number of structs:");
	//gets(Buf);
	fgets(Buf, sizeof(Buf), stdin);
	sscanf(Buf, "%d", pCount);
	
	// 1 or 2 structs - nope
	if ((*pCount) < 2)
		exit(0);
	
	// Allocate struct table
	*pPurTab = (SPurchase **) malloc(sizeof(SPurchase **) * (*pCount));
	
	// Get input for each struct
	for (p = 0; p < *pCount; p++)
	{
		// Alloc mem for struct
		(*pPurTab)[p] = (SPurchase *) calloc(sizeof(SPurchase), 1);
		Current = (*pPurTab)[p];
		printf("--- Type data for struct #%d of %d ---\n", p+1, *pCount);
		
		// Name
		printf("Type name (%d symbols max):\n", BUF_LEN);
		//gets(Current->Name);
		fgets(Current->Name, BUF_LEN, stdin);
		FixNewline(Current->Name, BUF_LEN);
		
		// Date
		puts("Type date in format \'day month year\':");
		//gets(Buf);
		fgets(Buf, sizeof(Buf), stdin);
		sscanf(Buf, "%d %d %d", &Day, &Mon, &Year);
		if (Day > 31)	Day = 31;
		if (Mon > 12)	Mon = 12;
		Current->Day = Day;
		Current->Year = Year;
		Current->Mon = Mon;
		
		// Cost
		puts("Type cost:");
		//gets(Buf);
		fgets(Buf, sizeof(Buf), stdin);
		sscanf(Buf, "%f", &Cost);
		Current->Cost = Cost;
		
		// Count
		puts("Type count:");
		//gets(Buf);
		fgets(Buf, sizeof(Buf), stdin);
		sscanf(Buf, "%d", &Num);
		Current->Num = Num;
	}
}

void DelPur(SPurchase *** pPurTab, int * pCount)
{
	int p;
	
	puts("[Freeing memory]");
	
	// Free each struct
	for (p = 0; p < *pCount; p++)
		free( (*pPurTab)[p] );
	
	// Free struct table
	free( (*pPurTab) );
	
	*pCount = 0;
}

int CmpPur(const void * ppvPur1, const void * ppvPur2)
{
	////int Year1, Year2;
	int Day1, Mon1, Day2, Mon2;
	SPurchase * pPur1;
	SPurchase * pPur2;
	
	// Read month and year
	pPur1 = *((SPurchase **) ppvPur1);
	pPur2 = *((SPurchase **) ppvPur2);
	Day1 = pPur1->Day;
	Mon1 = pPur1->Mon;
	////Year1 = pPur1->Year;
	Day2 = pPur2->Day;
	Mon2 = pPur2->Mon;
	////Year2 = pPur2->Year;
	
	// Decide who is bigger
	////if (Year1 > Year2)	// Year
	////	return 1;
	////else if (Year1 < Year2)
	////	return -1;
	////else
	{
		if (Mon1 > Mon2)	// Month
			return 1;
		else if (Mon1 < Mon2)
			return -1;
		else
		{
			if (Day1 > Day2)	// Day
				return 1;
			else if (Day1 < Day2)
				return -1;
			else
				return 0;
		}
	}
	
	// Prevent compiler warning
	return 0;
}

void SortPur(SPurchase ** PurTab, int Count)
{
	// Quicksort
	puts("[Sorting]");
	qsort(PurTab, Count, sizeof(SPurchase *), CmpPur);
}

void PrintPur(SPurchase ** PurTab, int Count)
{
	int p;
	
	puts("=== List of structures ===");
	
	// Get input for each struct
	for (p = 0; p < Count; p++)
	{
		printf("--- Struct #%d of %d ---\n", p+1, Count);
		
		// Name
		printf("Item name: %s\n", PurTab[p]->Name);
		
		// Date
		printf("Purchase date: %d.%d.%d\n", 
			PurTab[p]->Day, PurTab[p]->Mon, PurTab[p]->Year);
		
		// Cost
		printf("Item cost: %.2f\n", PurTab[p]->Cost);
		
		// Count
		printf("Item count: %d\n", PurTab[p]->Num);
	}
}

int main ()
{
	SPurchase ** PurTable = NULL;
	int PurCount = 0;
	
	puts("[main-start]");
	
	// Alloc and fill structures
	GetPur(&PurTable, &PurCount);
	
	// Print structires
	PrintPur(PurTable, PurCount);
	
	// Sort structures
	SortPur(PurTable, PurCount);
	
	// Print structures
	PrintPur(PurTable, PurCount);
	
	// Free memory
	DelPur(&PurTable, &PurCount);
	
	// Exit
	puts("[main-end]");
	return 0;
}
