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
//// LAB #4-2
////

// Includes
#include <stdio.h>	// printf, puts, scanf, gets
#include <string.h>	// strlen, strcpy
#include <stdlib.h>	// exit, malloc, calloc, free, qsort


// Definitions
#define BUF_LEN	256


// Prototypes


int main (int argc, char * argv[])
{
	FILE * fin;
	FILE * fout;
	char buf[BUF_LEN], symbol;
	int replaces;
	
	puts("[main-start]");
	
	puts("[getting args]");
	
	// Get arguments
	if (argc != 3)
	{
		puts("Use: [prog] [input_file] [num_replaces]");
		exit(0);
	}
	sscanf(argv[2], "%d", &replaces);
	
	puts("[opening files]");
	
	// Open input file
	fin = fopen(argv[1], "r");
	if (!fin)
	{
		puts("Can't open input file");
		exit(0);
	}
	
	// Open output file
	strcpy(buf, argv[1]);
	strcat(buf, ".out");
	fout = fopen(buf, "w");
	if (!fout) 
	{
		puts("Can't open output file");
		exit(0);
	}
	
	puts("[processing data]");
	
	// Process data
	while ( EOF != (symbol = fgetc(fin)) )
	{
		// Write symbol from input to output file
		fputc(symbol, fout);
		
		// Check symbol from input file
		if (symbol == ' ' && replaces > 0)
		{
			fputc(symbol, fout); // Put second space symbol in out file
			replaces--;
		}
	}
	
	puts("[closing files]");
	
	// Close files
	fclose(fin);
	fclose(fout);
	
	// Exit
	puts("[main-end]");
	return 0;
}
