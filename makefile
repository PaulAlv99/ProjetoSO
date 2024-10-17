CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lpthread
FICHEIROS = servidor.o ficheiro.o config.o
EXECUTAVEL = servidor

$(EXECUTAVEL):$(FICHEIROS)

clean:
	rm -f $(EXECUTAVEL) $(FICHEIROS)