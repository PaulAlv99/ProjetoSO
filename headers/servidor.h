#include "ficheiro.h"
#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 32

struct Servidor
{
    char ficheiroJogosESolucoesCaminho[PATH_SIZE];
    unsigned int portaServidor;
    unsigned int numeroConexoes;
    int tamanhoTabuleiro;
    int numeroLinhas;
    int numeroColunas;
    int numeroJogos;
};

struct ClienteInfo
{
    int idCliente;
    char tipoJogo[BUF_SIZE];
    char metodoResolucao[BUF_SIZE];
    int numeroTentativas;
    struct Jogo *jogoAtual;
};
// mudar
struct Jogo
{
    int idJogo;
    char jogo[81 + 1];
    char solucao[81 + 1];
};

void carregarConfigServidor(char *nomeFicheiro);
void carregarFicheiroJogosSolucoes(char *nomeFicheiro);
void ligacaoSocketS_C(int *server_fd, int *new_socket, struct sockaddr_in address);
void logEventoServidor(const char *message);
void logQueEventoServidor(int numero);
void lerMensagem(int *server_fd, int *new_socket);