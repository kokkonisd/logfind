CC = cc
CFLAGS = -Wall -g
VERSION = 1.1

all : logfind

install : logfind
	touch ~/.logfind
	cp logfind /usr/local/bin/

uninstall :
	rm -f ~/.logfind /usr/local/bin/logfind

logfind : logfind.o
	$(CC) $(CFLAGS) logfind.o -o logfind

logfind.o : logfind.c
	$(CC) $(CFLAGS) -c logfind.c

logfind.c : logfind.h

clean :
	rm -rf logfind logfind.o *.dSYM

.PHONY : all install uninstall deploy clean