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

char *atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas);

bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido);

void carregarFicheiroJogosSolucoes(char *nomeFicheiro);

struct ServidorConfig construtorServer(int dominio, int servico, int protocolo, __u_long interface, int porta, int backlog, char *ficheiroJogosESolucoesCaminho);

void *criaClienteThread(void *arg);

void iniciarServidorSocket(struct ServidorConfig *server);

void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, int nJogo, char *jogoADar, struct ServidorConfig serverConfig);

struct filaClientesSinglePlayer *criarFila();

void delete_queue(struct filaClientesSinglePlayer *fila);

bool boolEstaFilaCheia(struct filaClientesSinglePlayer *fila);

bool estaFilaCheiaTrSafe(struct filaClientesSinglePlayer *fila);

bool boolEstaFilaVazia(struct filaClientesSinglePlayer *fila);

bool estaFilaVaziaTrSafe(struct filaClientesSinglePlayer *fila);

bool enqueue(struct filaClientesSinglePlayer *fila, int clientID);

int dequeue(struct filaClientesSinglePlayer *fila);

void print_fila(struct filaClientesSinglePlayer *fila);

int getFilaTamanho(struct filaClientesSinglePlayer *fila);