signalServer:signalServer.o worker.o
	gcc -o $@ $^ -lpthread -g 

.SUFFIXES   :   .o .c
.c.o:
	gcc -g -c $*.c -o $*.o

clear:
	rm	*.o core.*

