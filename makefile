CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread
CLIENT_O = cliente.o
SERVER_O = servidor.o
EXECUTAVEIS = cliente servidor

# Rule to build both executables
all: $(EXECUTAVEIS)

# Rule to build the client executable
cliente: $(CLIENT_O)
	$(CC) $(CLIENT_O) -o cliente $(LDFLAGS)

# Rule to build the server executable
servidor: $(SERVER_O)
	$(CC) $(SERVER_O) -o servidor $(LDFLAGS)

# Rules to build object files
cliente.o: cliente.c
	$(CC) $(CFLAGS) -c cliente.c

servidor.o: servidor.c
	$(CC) $(CFLAGS) -c servidor.c

# Clean rule to remove the executables and object files
clean:
	rm -f $(EXECUTAVEIS) $(CLIENT_O) $(SERVER_O)
