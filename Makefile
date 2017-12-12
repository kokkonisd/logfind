CC = cc
CFLAGS = -Wall -g

all: logfind

logfind : logfind.o
	$(CC) $(CFLAGS) logfind.o -o logfind

logfind.o : logfind.c
	$(CC) $(CFLAGS) -c logfind.c

clean:
	rm -rf logfind logfind.o *.dSYM *.o