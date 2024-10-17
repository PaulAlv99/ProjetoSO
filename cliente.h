
#include "ficheiro.h"
#include "config.h"

//1KB
#define BUF_SIZE 128
#define PATH_SIZE 16 
#define IP_SIZE 16
#define CAPACIDADE_CONFSERVER 1
#define TAMANHO_TABULEIRO 9
#define NUMEROS_NO_JOGO 81


void imprimirTabuleiro(char* jogo);
void carregarConfigCliente(char* nomeFicheiro);

struct ClienteConfig{
    int idCliente;
    char ipServidor[IP_SIZE];
};

struct ClienteThread{
    struct ClienteConfig clienteConfig;
    char tabuleiro[NUMEROS_NO_JOGO];
};

