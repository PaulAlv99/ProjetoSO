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

struct ThreadCliente
{
    int socketCliente;
    struct ServidorConfig *server;
    int clienteId;
};

struct ThreadMsgArgs
{
    int socketCliente;
    struct ClienteConfig *clienteConfig;
    struct ServidorConfig *serverConfig;
    pthread_mutex_t *mutexMsg;
    bool *running;
    char *jogoADar;
    int nJogo;
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
    struct SalaSinglePlayer *sala;
};

struct SalaSinglePlayer
{
    int idSala;
    int clientesMax;
    int clienteMin;
    int nClientes;
    char *clientesID;
    struct Jogo jogo;
    pthread_mutex_t mutexSala;
};

struct filaClientesSinglePlayer
{
    int *clientesID;
    int front;
    int rear;
    int tamanho;
    int capacidade;
    pthread_mutex_t mutex;
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
void *criaClienteThread(void *arg);
void *criaClienteThreadEnviarMSG(void *arg);
void *criaClienteThreadReceberMSG(void *arg);
void iniciarServidorSocket(struct ServidorConfig *server);
void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, int nJogo, char *jogoADar, struct ServidorConfig serverConfig);
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);
void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas);
void resolverJogoParcial(char jogo[], char solucao[], int nTentativas);
void imprimirTabuleiro(char jogo[]);
struct filaClientesSinglePlayer *criarFila();
void delete_queue(struct filaClientesSinglePlayer *queue);
bool bolEstaFilaCheia(struct filaClientesSinglePlayer *queue);
bool boolFilaVazia(struct filaClientesSinglePlayer *queue);
bool estaFilaCheiaTrS(struct filaClientesSinglePlayer *queue);
bool estaFilaVaziaTrSafe(struct filaClientesSinglePlayer *queue);
void print_fila(struct filaClientesSinglePlayer *queue);
int getTamanhoFila(struct filaClientesSinglePlayer *queue);
bool enqueue(struct filaClientesSinglePlayer *queue, int value);
int dequeue(struct filaClientesSinglePlayer *queue);
void processarNovoJogo(struct ClienteConfig *config, char *jogoADar, int nJogo);
void processarAtualizacaoJogo(struct ClienteConfig *config, int nJogo);
// extern void logEventoCliente(const char *message);
// extern void logQueEventoCliente(int numero);