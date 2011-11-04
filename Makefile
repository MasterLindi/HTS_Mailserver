#Makefile Server,Client

all: client server

client: clientmain.o clientfunctions.o
	gcc -g -O -o client clientmain.o clientfunctions.o

clientmain.o: clientmain.c clientfunctions.h
	gcc -g -O -c clientmain.c

clientfunctions.o: clientfunctions.c clientfunctions.h
	gcc -g -O -c clientfunctions.c

server: servermain.o serverfunctions.o
	gcc -g -O -o server servermain.o serverfunctions.o -lpthread

servermain.o: servermain.c serverfunctions.h
	gcc -g -O -c servermain.c

serverfunctions.o: serverfunctions.c serverfunctions.h
	gcc -g -O -c serverfunctions.c

clean: 
	rm -f servermain.o serverfunctions.o server clientmain.o clientfunctions.o client
