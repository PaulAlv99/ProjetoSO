#include "util.h"

//1KB
#define IP_SIZE 16

void imprimirTabuleiro(char* jogo);
void carregarConfigCliente(char* nomeFicheiro);

//Descomentar depois de apagar o mesmo em servidor.h
// struct ClienteConfig{
//     unsigned long idCliente;
//     char ipServidor[IP_SIZE];
// };

// struct ClienteThread{
//     struct ClienteConfig clienteConfig;
//     char tabuleiro[NUMEROS_NO_JOGO];
// };

