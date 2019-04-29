CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g

transport: transport.c communication.o
	${CC} -o transport transport.c communication.o ${CFLAGS}

communication.o: communication.c
	${CC} -c communication.c ${CFLAGS}

clean:
	rm -rf *.o

distclean: clean
	rm -rf transport