CC = gcc
CPPFLAGS=
CFLAGS = -Wall -std=c11 -g -pedantic -Iinclude
LOCATION = $(shell pwd)
LDFLAGS= -L./bin

all:list parser

mainTestSuit: mainTestSuit.o libcal.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/mainTestSuit bin/mainTestSuit.o   -lcal -llist

mainTestSuit.o: src/mainTestsuit.c include/testsuit.h include/HelperFunctions.h
	$(CC) $(CFLAGS) -c src/mainTestsuit.c -o bin/mainTestSuit.o

parser:libcal.so


libcal.so: CalendarParser.o HelperFunctions.o LinkedListAPI.o
	$(CC) -shared -o ../libcal.so ./bin/CalendarParser.o ./bin/HelperFunctions.o bin/LinkedListAPI.o

CalendarParser.o: src/CalendarParser.c include/CalendarParser.h include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/CalendarParser.c -o bin/CalendarParser.o

HelperFunctions.o: src/HelperFunctions.c include/HelperFunctions.h include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/HelperFunctions.c -o bin/HelperFunctions.o

list:liblist.so

liblist.so: LinkedListAPI.o
	$(CC) -shared -o bin/liblist.so bin/LinkedListAPI.o

LinkedListAPI.o: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c -o bin/LinkedListAPI.o
clean:
	rm -rf bin/*.o bin/*.so

