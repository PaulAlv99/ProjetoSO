#include "ficheiro.h"

// 1KB
#define IP_SIZE 16
#define PORTA_SIZE 16

struct JogoAtual
{
    int idJogo;
    char jogo[81 + 1];
    char *tempoInicio;
    char tempoFinal[TEMPO_TAMANHO];
    int numeroTentativas;
};

struct Cliente
{
    int idCliente;
    char tipoJogo[BUF_SIZE];
    char metodoResolucao[BUF_SIZE];
    char ipServidor[IP_SIZE];
    unsigned int portaServidor;
    int numeroLinhas;
    int numeroColunas;
    struct JogoAtual *jogoAtual;
};

void carregarConfigCliente(char *nomeFicheiro);
void logEventoCliente(const char *message);
void logQueEventoCliente(int numero);
char *mandarDadosCliente_Jogo();
void imprimirTabuleiro(char *jogo);
