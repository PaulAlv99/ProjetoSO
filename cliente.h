#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>


void imprimirTabuleiro(char* jogo);
void carregarConfigCliente(char* nomeFicheiro);

struct ClienteConfig{
    int idCliente;
    char ipServidor[IP_SIZE];
};


