CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lpthread

all: cliente servidor

cliente: ./cliente/cliente.o ./cliente/clienteConfig.o ./cliente/clienteLogger.o ./util/ficheiro.o ./util/config.o
	$(CC) -g -o ./cliente/cliente ./cliente/cliente.o ./cliente/clienteConfig.o ./cliente/clienteLogger.o ./util/ficheiro.o ./util/config.o $(LDFLAGS)

servidor: ./servidor/servidor.o ./servidor/servidorConfig.o ./servidor/servidorLogger.o ./util/ficheiro.o ./util/config.o
	$(CC) -g -o ./servidor/servidor ./servidor/servidor.o ./servidor/servidorConfig.o ./servidor/servidorLogger.o ./util/ficheiro.o ./util/config.o $(LDFLAGS)

clean:
	rm -f ./cliente/*.o ./servidor/*.o ./util/*.o