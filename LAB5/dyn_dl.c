/*
=====================================================================
Copyright (c) 2017-2019, Alexey Leushin
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

//// 03.10.19
//// LAB #5 - dlfcn dynamic library import
////

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>	// dl library for on-the-fly shared object loading
#include "lib.h"	// Import library function prototypes

// Defines
#define BUF_LEN 100

// Protoypes
double UTIL_ScanNum();	// Gets double from keyboard

double UTIL_ScanNum()
{
	char Buf[BUF_LEN];
	double Num;
	
	puts("Input number:");
	fgets(Buf, sizeof(Buf), stdin);
	sscanf(Buf, "%lf", &Num);
	
	return Num;
}

int main()
{
	void * plib; 		// For dl
	fnp_pow3 dl_pow3;	// Pointer for imported pow3 func
	fnp_pow4 dl_pow4;	// Pointer for imported pow4 func
	double x;
	
	puts("[dl dynamic library example: start]");
	
	// Open library
	puts("[dl: opening library]");
	plib = dlopen("./libdyn.so", RTLD_LAZY);
	if (!plib)
	{
		// Print error and stop
		printf("dlopen error: %s\n", dlerror());
		exit(1);
	};
	
	// Get functionns
	puts("[dl: reading functions]");
	dl_pow3 = dlsym(plib, LIB_POW3);
	dl_pow4 = dlsym(plib, LIB_POW4);
	
	// Get number from keyboard
	puts("[getting number from user]");
	x = UTIL_ScanNum();
	
	// Calculate and print powers
	printf("%g ^ 3 = %g\n", x, dl_pow3(x));
	printf("%g ^ 4 = %g\n", x, dl_pow4(x));
	
	// Close library
	puts("[dl: closing library]");
	dlclose(plib);
	
	// Exit
	return 0;
}