#Makefile to compile the code
#

CC=gcc
#CFLAGS=-Wall -g
shell: shell.c
	$(CC) -o shell shell.c
clean:
	$(RM) shell

