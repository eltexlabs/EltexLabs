all: serv.elf cl1.elf cl2.elf

serv.elf: serv.c shared.o
	gcc -std=c99 -Wall -o serv.elf serv.c shared.o

cl1.elf: cl1.c shared.o
	gcc -std=c99 -Wall -o cl1.elf cl1.c shared.o

cl2.elf: cl2.c shared.o
	gcc -std=c99 -Wall -o cl2.elf cl2.c shared.o

shared.o: shared.c
	gcc -std=c99 -Wall -c -o shared.o shared.c
