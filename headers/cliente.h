#include "ficheiro.h"

//1KB
#define IP_SIZE 16

void imprimirTabuleiro(char* jogo);
void carregarConfigCliente(char* nomeFicheiro);

struct ClienteConfig{
    unsigned long idCliente;
    char ipServidor[IP_SIZE];
};

struct ClienteThread{
    struct ClienteConfig clienteConfig;
    char tabuleiro[NUMEROS_NO_JOGO];
};

extern void logEventoCliente(const char* message);
extern void logQueEventoCliente(int numero);
