#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 32
#define CONFIGFILE "./configs/servidor.conf"
#include "util.h"
#include "../headers/cliente.h"

// So para teste apagar depois
#define IP_SIZE 16

// Isto nao e daqui, apagar depois
struct Jogo
{
    int idJogo;
    char jogo[NUMEROS_NO_JOGO + 1];
    char solucao[NUMEROS_NO_JOGO + 1];
};

struct ClienteThread
{
    struct ClienteConfig clienteConfig;
    char tabuleiro[NUMEROS_NO_JOGO + 1];
};

struct ServidorConfig
{
    int dominio;
    int servico;
    int protocolo;
    __u_long interface;
    unsigned int porta;
    int backlog;
    char ficheiroJogosESolucoesCaminho[PATH_SIZE];
};
void carregarConfigServidor(char *nomeFicheiro, struct ServidorConfig *serverConfig);
void logEventoServidor(const char *message);
void logQueEventoServidor(int numero, int clienteID);
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]);
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);
char *atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas);
// void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas);
void carregarFicheiroJogosSolucoes(char *nomeFicheiro);
struct ServidorConfig construtorServer(int domain, int service, int protocol, __u_long interface, int port, int backlog, char *ficheiroJogosESolucoesCaminho);
void iniciarServidorSocket(struct ServidorConfig *server);
void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, int nJogo, char *jogoADar);
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas);
void resolverJogoParcial(char jogo[], char solucao[], int nTentativas);
void imprimirTabuleiro(char jogo[]);
// extern void logEventoCliente(const char *message);
// extern void logQueEventoCliente(int numero);