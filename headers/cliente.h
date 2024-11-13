#include "util.h"

// 1KB
#define IP_SIZE 16

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
// struct ClienteThread{
//     struct ClienteConfig clienteConfig;
//     char tabuleiro[NUMEROS_NO_JOGO];
// };
void tentarSolucaoCompleta(char *jogo, char *valoresCorretos);
void logEventoCliente(const char *message, struct ClienteConfig *clienteConfig);
void logQueEventoCliente(int numero, struct ClienteConfig clienteConfig);
void imprimirTabuleiro(char *jogo);
void carregarConfigCliente(char *nomeFicheiro, struct ClienteConfig *clienteConfig);
void mandarETratarMSG(struct ClienteConfig *clienteConfig);
void iniciarClienteSocket(struct ClienteConfig *clienteConfig);
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]);