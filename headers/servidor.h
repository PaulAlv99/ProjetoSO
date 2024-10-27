#include "ficheiro.h"
#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 32
#define SERVER_PORT 25575

struct ServidorConfig
{
    char ficheiroJogosESolucoesCaminho[PATH_SIZE];
};

struct ClienteInfo
{
    int idCliente;
    char tipoJogo[BUF_SIZE];
    char metodoResolucao[BUF_SIZE];
    int numeroTentativas;
    struct Jogo *jogoAtual;
};

struct Jogo
{
    int idJogo;
    char jogo[NUMEROS_NO_JOGO + 1];
    char solucao[NUMEROS_NO_JOGO + 1];
};

void carregarConfigServidor(char *nomeFicheiro);
void logEventoServidor(const char *message);
void logQueEventoServidor(int numero);
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]);
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);
void atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas);
void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas);
void carregarFicheiroJogosSolucoes(char *nomeFicheiro);
void enviarJogo(int new_socket);
void ligacaoSocketS_C(int *server_fd, int *new_socket, struct sockaddr_in address);
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas);
void resolverJogoParcial(char jogo[], char solucao[], int nTentativas);
void imprimirTabuleiro(char jogo[]);