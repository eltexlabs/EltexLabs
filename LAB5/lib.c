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

//// 30.09.19
//// LAB #5 - library
////

// Defines
//#define USE_MATH_LIB

// Includes
#ifdef USE_MATH_LIB
 #include <math.h>
#endif
#include <stdio.h>
#include "lib.h"

double pow3(double x)
{
	puts("[library: pow3 func called]");
	#ifdef USE_MATH_LIB
	 return pow(x, 3.0);
	#else
	 return x*x*x;
	#endif
}

double pow4(double x)
{
	puts("[library: pow4 func called]");
	#ifdef USE_MATH_LIB
	 return pow(x, 4.0);
	#else
	 return x*x*x*x;
	#endif
}

// Init and deinit funcs for dlopen
/*void _init()
{
	puts("[library init func called]");
}

void _fini()
{
	puts("[library deinit func called]");
}*/
