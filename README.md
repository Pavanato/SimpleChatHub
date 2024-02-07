# SimpleChatHub

This is a simple chat server implemented in C++. It uses sockets for communication and supports multiple clients.

## Features

- Multithreaded server that can handle multiple clients simultaneously
- Each client runs in its own thread
- Clients can send messages to all other clients
- Server broadcasts messages from clients to all other clients
- Server keeps track of all connected clients

## Building the Project

You can build the project using the provided Makefile:

```sh
make
```

This will compile the server code into an executable named server.

## Running the Server

You can start the server by running the server executable:

```sh
./server
```

The server will start listening for client connections on ```port 12345```.

## Connecting to the Server

Clients can connect to the server using any TCP/IP client, such as telnet:

```sh
telnet localhost 12345
```

Once connected, the server will prompt the client to enter their name. After that, the client can start sending messages to the server.
