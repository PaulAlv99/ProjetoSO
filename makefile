CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lpthread
FICHEIROS = servidor.o ficheiro.o config.o cliente.o
EXECUTAVEL = servidor

$(EXECUTAVEL):$(FICHEIROS)

clean:
	rm -f $(EXECUTAVEL) $(FICHEIROS)