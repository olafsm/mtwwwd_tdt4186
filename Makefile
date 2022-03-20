CC=gcc
CFLAGS= -I

make: *.c
	$(CC) -pthread -g -o mtwwwd *.c 

clean:
	-rm -f *.o

purge:
	-rm -f ./mtwwwd