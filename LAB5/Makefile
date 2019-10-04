### lab #5 makefile ###

all: libstat.a stat.elf libdyn.so dyn.elf dyn_dl.elf



### Make dl dynamic library example ###

# -ldl - use ld library
dyn_dl.elf: dyn_dl.c
	gcc -Wall -o dyn_dl.elf dyn_dl.c -ldl



### Make normal dynamic library example ###

#LD_LIBRARY_PATH - custom library path (default: /usr/lib)
# -ldyn - use created dynamic library
dyn.elf: dyn.c
	LD_LIBRARY_PATH=. gcc -Wall -o dyn.elf dyn.c -L. -ldyn



### Make static library example ###

# -L. - search libs in current directory
# -lstat - library name without "lib" and ".a" of created static library
# -lm - use math library to fix "undefined reference to `pow'" messages
#	https://stackoverflow.com/questions/8671366/undefined-reference-to-pow-and-floor
stat.elf: stat.c
	gcc -Wall -o stat.elf stat.c -L. -lstat -lm



### Make dynamic library ###

# -lm - use math library
libdyn.so: libpic.o
	gcc -o libdyn.so -shared libpic.o -lm

# -fPIC - Position Independent Code
libpic.o: lib.c
	gcc -o libpic.o -c -Wall lib.c -fPIC



### Make static library ###

# c - create lib file if not present
# r - replace object files inside lib
libstat.a: lib.o
	ar rc libstat.a lib.o
	ranlib libstat.a

lib.o: lib.c
	gcc -c -Wall lib.c
