#define CAPACIDADE_CONFSERVER 1
#define CONFIGFILE "./configs/servidor.conf"
#include "util.h"

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

struct SalaSinglePlayer
{
    int idSala;
    int clientesMax;
    int clienteMin;
    int nClientes;
    bool jogadorAResolver;
    pthread_mutex_t mutexSala;
    struct Jogo jogo;
    int clienteAtualID; // Add this to track current client
};

struct filaClientesSinglePlayer
{
    int *clientesID;
    int front;
    int rear;
    int tamanho;
    int capacidade;
    pthread_mutex_t mutex;
    sem_t customers;
};

struct FormatoMensagens
{
    char *idCliente;
    char *tipoJogo;
    char *tipoResolucao;
    char *temJogo;
    char *idJogo;
    char *jogo;
    char *valoresCorretos;
    char *tempoInicio;
    char *tempoFinal;
    char *resolvido;
    char *numeroTentativas;
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

void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, struct ServidorConfig serverConfig);

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