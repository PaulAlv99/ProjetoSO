# Compiler e flags
CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

# Objectos
CLIENT_O = cliente.o
SERVER_O = servidor.o

# Diretorios
CLIENTE_DIR := cliente
SERVIDOR_DIR := servidor

# caminho dos executaveis
CLIENT_EXEC = $(CLIENTE_DIR)/cliente
SERVER_EXEC = $(SERVIDOR_DIR)/servidor

all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Regras para criar executavel cliente
$(CLIENT_EXEC): $(CLIENT_O)
	mkdir -p $(CLIENTE_DIR)
	$(CC) $(CLIENT_O) -o $@ $(LDFLAGS)

# Regras para criar executavel server
$(SERVER_EXEC): $(SERVER_O)
	mkdir -p $(SERVIDOR_DIR)
	$(CC) $(SERVER_O) -o $@ $(LDFLAGS)

# Regras para criar objetos
$(CLIENT_O): cliente.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_O): servidor.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remover executaveis,diretorios e objetos
clean:
	rm -rf $(CLIENTE_DIR) $(SERVIDOR_DIR) $(CLIENT_O) $(SERVER_O)
