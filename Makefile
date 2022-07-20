##
# multiServer
#
# @file
# @version 0.1
CC = g++ -std=c++11
CFLAGS = -Wall

multiserver.o: multiserver.cpp
	$(CC) $(CFLAGS) -g -c multiserver.cpp

cSock.o: cSock.cpp cSock.h
	$(CC) $(CFLAGS) -c cSock.cpp

test: clean multiserver.o cSock.o
	$(CC) $(CFLAGS) -o test multiserver.o cSock.o

run: multiserver
	./multiserver

clean:
	rm -rf *.o *.a multiserver test server Client servertest


# end