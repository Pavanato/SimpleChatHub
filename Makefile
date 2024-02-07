# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -std=c++11

# Targets
all: server

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp

run: server
	./server

clean:
	rm -f server