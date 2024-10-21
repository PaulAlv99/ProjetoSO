CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lpthread
FICHEIROS = servidor.o ficheiro.o config.o cliente.o
EXECUTAVEL = servidor
CLIENTE_DIR := cliente
SERVIDOR_DIR := servidor

$(EXECUTAVEL):$(FICHEIROS)

clean:
	rm -rf $(EXECUTAVEL) $(FICHEIROS) $(CLIENTE_DIR)