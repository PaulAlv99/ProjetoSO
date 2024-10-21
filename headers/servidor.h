#include "ficheiro.h"
#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 32

struct ServidorConfig{
    char ficheiroJogosESolucoesCaminho[PATH_SIZE];
};


struct Jogo{
    long idJogo;
    char jogo[NUMEROS_NO_JOGO+1];
    char solucao[NUMEROS_NO_JOGO+1];
};

struct JogoAtual{
    long idJogo;
    char jogo[NUMEROS_NO_JOGO];
    char tempoInicio[TEMPO_TAMANHO];
    char tempoFinal[TEMPO_TAMANHO];
    long numeroTentativas;
};

void carregarConfigServidor(char* nomeFicheiro);
void logEventoServidor(const char* message) ;
void logQueEventoServidor(int numero);
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]);
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);
void atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas);
void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas);
void carregarFicheiroJogosSolucoes(char* nomeFicheiro);

bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas);
void resolverJogoParcial(char jogo[], char solucao[], int nTentativas);
void imprimirTabuleiro(char jogo[]);