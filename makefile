# Compiler and flags
CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

# Object files
CLIENT_O = cliente.o
SERVER_O = servidor.o

# Executable names with relative paths
CLIENT_EXEC = ./cliente/cliente
SERVER_EXEC = ./servidor/servidor

# Default target to build both executables
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Rule to build the client executable
$(CLIENT_EXEC): $(CLIENT_O)
	$(CC) $(CLIENT_O) -o $@ $(LDFLAGS)

# Rule to build the server executable
$(SERVER_EXEC): $(SERVER_O)
	$(CC) $(SERVER_O) -o $@ $(LDFLAGS)

# Rules to build object files
$(CLIENT_O): cliente.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_O): servidor.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove the executables and object files
clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(CLIENT_O) $(SERVER_O)
