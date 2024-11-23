#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define CHECK_NULL(x) ((x) == NULL)

#define TEMPO_TAMANHO 22
#define BUF_SIZE 8192
// 81 + \0
#define NUMEROS_NO_JOGO 81
#define NUM_LINHAS 9
#define NUM_COLUNAS 9
#define NUM_JOGOS 3
#define INFO_SIZE 256
#define SHM_SIZE 1024
#define NUM_MAX_CLIENTES_FILA_SINGLEPLAYER 200
#define IP_SIZE 16
#define PATH_SIZE 32

struct JogoAtual
{
    int idJogo;
    char jogo[NUMEROS_NO_JOGO + 1];
    char valoresCorretos[NUMEROS_NO_JOGO + 1];
    char tempoInicio[TEMPO_TAMANHO];
    char tempoFinal[TEMPO_TAMANHO];
    bool resolvido;
    int numeroTentativas;
};

struct ClienteConfig
{
    char TemJogo[INFO_SIZE];
    int idCliente;
    char tipoJogo[INFO_SIZE];
    char tipoResolucao[INFO_SIZE];
    char ipServidor[IP_SIZE];
    int dominio;
    unsigned int porta;
    __u_long interface;
    int socket;
    struct JogoAtual jogoAtual;
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

// globais
const char *getTempo();
const char *getTempoHoraMinutoSegundo();
void enviarMensagens();
void receberMensagens(int nUtilizadores);
int fecharFicheiro(FILE *file);
FILE *abrirFicheiroRead(char *filename);
int validarNomeFile(char *arquivoNome, char *padrao);