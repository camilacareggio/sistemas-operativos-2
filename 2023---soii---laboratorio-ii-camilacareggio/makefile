CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
LFLAGS = -lz

clientA: clientA.o
	gcc -o clientA clientA.o

clientA.o: clientA.c
	gcc $(CFLAGS) -c clientA.c

clientB: clientB.o
	gcc -o clientB clientB.o $(LFLAGS)

clientB.o: clientB.c
	gcc $(CFLAGS) -c clientB.c

clientC: clientC.o
	gcc -o clientC clientC.o

clientC.o: clientC.c
	gcc $(CFLAGS) -c clientC.c

server: server.o cJSON.o
	gcc -o server server.o cJSON.o $(LFLAGS)

server.o: server.c
	gcc $(CFLAGS) -c server.c

cJSON.o: cJSON.c cJSON.h
	gcc $(CFLAGS) -c cJSON.c

clean:
	rm *.o *.gz clientA clientB clientC server