#include "util.h"
struct MultiplayerState {
    int currentRound;
    bool roundActive;
    pthread_mutex_t stateLock;
    bool canAttempt;
    bool waitingForResponse;
};
// 1KB
#define IP_SIZE 16

void carregarConfigCliente(char *nomeFicheiro, struct ClienteConfig *clienteConfig);

void imprimirTabuleiro(char *jogo);

void logEventoCliente(const char *message, struct ClienteConfig *clienteConfig);

void logQueEventoCliente(int numero, struct ClienteConfig clienteConfig);

void construtorCliente(int dominio, unsigned int porta, __u_long interface, struct ClienteConfig *clienteConfig);

void iniciarClienteSocket(struct ClienteConfig *clienteConfig);

void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);

void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]);

void mandarETratarMSG(struct ClienteConfig *clienteConfig);

bool desistirDeResolver();