CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lcjson -lpthread
FICHEIROS = servidor.o ficheiro.o config.o cliente.o
EXECUTAVEL = servidor
CLIENTE_DIR := cliente
SERVIDOR_DIR := servidor

all: cliente servidor

cliente: cliente.o ficheiro.o config.o
	$(CC) -g -o cliente cliente.o ficheiro.o config.o $(LDFLAGS)

servidor: servidor.o ficheiro.o config.o
	$(CC) -g -o servidor servidor.o ficheiro.o config.o $(LDFLAGS)

clean:
	rm -f *.o *~ cliente servidor