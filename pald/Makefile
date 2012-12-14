CC=gcc
CFLAGS=-Wall -g -Werror
LDFLAGS=-lcrypt
all: pald

pald: pald.o
	$(CC) pald.o -o pald $(LDFLAGS) 
pald.o: pald.c
	$(CC) -c pald.c $(CFLAGS)

clean:
	rm -rf *.o pald