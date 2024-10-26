#include "ficheiro.h"

// 1KB
#define IP_SIZE 16
#define PORTA_SIZE 16
void imprimirTabuleiro(char *jogo);
void carregarConfigCliente(char *nomeFicheiro);

struct ClienteConfig
{
    int idCliente;
    char tipoJogo[BUF_SIZE];
    char metodoResolucao[BUF_SIZE];
    char ipServidor[IP_SIZE];
    unsigned int portaServidor;
};

struct JogoAtual
{
    int idJogo;
    char jogo[NUMEROS_NO_JOGO + 1];
    char *tempoInicio;
    char tempoFinal[TEMPO_TAMANHO];
    long numeroTentativas;
};

extern void logEventoCliente(const char *message);
extern void logQueEventoCliente(int numero);
