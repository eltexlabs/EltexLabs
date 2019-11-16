all: serv.elf cl1.elf cl2.elf

serv.elf: serv.c shared.o msg.pb-c.o
	gcc -std=c99 -Wall -o serv.elf serv.c shared.o msg.pb-c.o -lprotobuf-c

cl1.elf: cl1.c shared.o msg.pb-c.o
	gcc -std=c99 -Wall -o cl1.elf cl1.c shared.o msg.pb-c.o -lprotobuf-c

cl2.elf: cl2.c shared.o msg.pb-c.o
	gcc -std=c99 -Wall -o cl2.elf cl2.c shared.o msg.pb-c.o -lprotobuf-c

shared.o: shared.c
	gcc -std=c99 -Wall -c -o shared.o shared.c

msg.pb-c.o: msg.pb-c.c
	gcc -std=c99 -Wall -c -o msg.pb-c.o msg.pb-c.c

msg.pb-c.c: msg.proto
	protoc-c --c_out=. msg.proto

clean:
	rm shared.o
	rm serv.elf
	rm cl1.elf
	rm cl2.elf
	rm msg.pb-c.c
	rm msg.pb-c.h
	rm msg.pb-c.o
