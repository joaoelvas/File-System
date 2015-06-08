CFLAGS= -Wall -g
all: fs-shell

fs-shell: shell.o fs.o disk.o
	gcc $(CFLAGS) -o fs-shell shell.o fs.o disk.o -lm
	
shell.o: shell.c
	gcc $(CFLAGS) shell.c -c -o shell.o 

fs.o: fs.c fs.h
	gcc $(CFLAGS) fs.c -c -o fs.o

disk.o: disk.c disk.h
	gcc $(CFLAGS) disk.c -c -o disk.o

clean:
	rm fs-shell disk.o fs.o shell.o
