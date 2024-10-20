#include "ficheiro.h"
#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 16

struct ServidorConfig{
    char ficheiroJogosCaminho[PATH_SIZE];
    char ficheiroSolucoesCaminho[PATH_SIZE];
};

void carregarConfigServidor(char* nomeFicheiro);
void logEventoServidor(const char* message) ;
void logQueEventoServidor(int numero);
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]);
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);

void atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[]);
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoCompleto(char jogo[], char tentativaAtual[], char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoParcial(char jogo[], char tentativaAtual[], char valoresCorretos[], char solucao[], bool resolvido);

void imprimirTabuleiro(char jogo[]);