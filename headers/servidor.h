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


//podia ser mudado para struct do cliente mesmo
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
    sem_t jogadorFinalizou;
    sem_t salaPronta;
    int socketCliente;
    struct Jogo jogo;
    struct ClienteConfig clienteAtual; // Add this to track current client
};
typedef struct {
    char buffer[BUF_SIZE];  // Shared buffer for game state
    bool bufferFull;        // Flag indicating if buffer is full
    sem_t mutex;           // Protects access to buffer
    sem_t empty;           // Counts empty buffer slots 
    sem_t full;            // Counts full slots
} GameBuffer;
struct SalaMultiplayer {
    int idSala;
    int clientesMax;
    int clienteMin;
    int nClientes;
    int winnerID;
    int esperandoEntrar;
    int* clienteAtualID;
    bool jogoIniciado;
    bool temDeEsperar;
    struct Jogo jogo;
    bool hasWinner;
    pthread_mutex_t winnerMutex;
    sem_t sinalizarVencedor;
   
    struct ClienteConfig* clientes;
};
struct filaClientesSinglePlayer
{
    struct ClienteConfig *cliente;
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
    char* idSala;
    char *jogo;
    char *valoresCorretos;
    char *tempoInicio;
    char *tempoFinal;
    char *resolvido;
    char *numeroTentativas;
};
enum GameState {
    WAITING_PLAYERS,
    GAME_STARTING,
    GAME_RUNNING,
    GAME_ENDED
};
// inicializar e configs
void carregarConfigServidor(char *nomeFicheiro, struct ServidorConfig *serverConfig);
void carregarFicheiroJogosSolucoes(char *nomeFicheiro, struct Jogo jogosEsolucoes[]);
void construtorServer(struct ServidorConfig *servidor, int dominio, int servico, int protocolo, __u_long interface, int porta, int backlog, char *ficheiroJogosESolucoesCaminho);
void iniciarServidorSocket(struct ServidorConfig *server, struct Jogo jogosEsolucoes[]);

// funcoes de logs
void logEventoServidor(const char *message);
void logQueEventoServidor(int numero, int clienteID,int salaID);

// logica de jogo
char *atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas);
bool verificaResolvido(char valoresCorretos[], char solucao[]);

// tratar clientes
void *criaClienteThread(void *arg);
void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, struct ServidorConfig serverConfig);

// funcoes singleplayer
struct SalaSinglePlayer* handleSinglePlayerFila(struct ClienteConfig *cliente, struct ServidorConfig* serverConfig);
void *SalaSinglePlayer(void *arg);
struct SalaSinglePlayer *criarSalaSinglePlayer(int idSala);
void *iniciarSalaSinglePlayer(void *arg);
void iniciarSalasJogoSinglePlayer(struct ServidorConfig *serverConfig, struct Jogo jogosEsolucoes[]);

//funcoes multiplayer
void *iniciarSalaMultiplayerFaster(void *arg);
void iniciarSalasJogoMultiplayer(struct ServidorConfig *serverConfig, struct Jogo jogosEsolucoes[]);
// funcoes fila
struct filaClientesSinglePlayer *criarFila(struct ServidorConfig *serverConfig);
void delete_queue(struct filaClientesSinglePlayer *fila);
bool enqueue(struct filaClientesSinglePlayer *fila, struct ClienteConfig cliente);
struct ClienteConfig dequeue(struct filaClientesSinglePlayer *fila);
bool boolEstaFilaCheia(struct filaClientesSinglePlayer *fila);
bool estaFilaCheiaTrSafe(struct filaClientesSinglePlayer *fila);
bool boolEstaFilaVazia(struct filaClientesSinglePlayer *fila);
bool estaFilaVaziaTrSafe(struct filaClientesSinglePlayer *fila);
void print_fila(struct filaClientesSinglePlayer *fila);
int getFilaTamanho(struct filaClientesSinglePlayer *fila);

//funcoes uteis para parses e afins 
void updateClientConfig(struct ClienteConfig *clienteConfig, const struct FormatoMensagens *msg, const char *jogoADar, int nJogo,int salaID);
void bufferParaStructCliente(char *buffer, const struct ClienteConfig *clienteConfig);
char *handleResolucaoJogoSIG(struct ClienteConfig *clienteConfig, struct SalaSinglePlayer *sala);
struct FormatoMensagens parseMensagem(char *buffer);
bool validarMensagemVazia(struct FormatoMensagens *msg);

bool verSeJogoAcabouEAtualizar(struct ClienteConfig *cliente, struct SalaSinglePlayer *sala);
bool verSeJogoAcabouEAtualizarMultiplayerFaster(struct ClienteConfig *cliente, struct SalaMultiplayer *sala);