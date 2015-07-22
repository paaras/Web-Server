CC=gcc

all: serverfork.c
	gcc serverfork.c -o serverfork

clean: 
	rm -rf *o serverfork
