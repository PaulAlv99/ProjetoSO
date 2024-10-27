#include "../headers/cliente.h"
struct ClienteConfig clienteConfig;

// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;

void logEventoCliente(const char *message)
{
    // modo append
    char str[BUF_SIZE];
    char *clienteLogsDir = "clienteLogs";
    char *nomeFicheiro = "LogCliente";
    char *tipoFicheiro = ".txt";
    // se o id for muito alto mesmo dá problema overflow mas nao dá crash por overflow
    snprintf(str, BUF_SIZE, "%s/%s%u%s", clienteLogsDir, nomeFicheiro, clienteConfig.idCliente, tipoFicheiro);
    pthread_mutex_lock(&mutexClienteLog);
    FILE *file = fopen(str, "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexClienteLog);
        return;
    }
    fprintf(file, "[%s] [Cliente ID: %u] %s\n", getTempo(), clienteConfig.idCliente, message);

    fclose(file);
    pthread_mutex_unlock(&mutexClienteLog);
}

void logQueEventoCliente(int numero)
{
    switch (numero)
    {
    case 1:
        logEventoCliente("Iniciou");
        break;
    case 2:
        logEventoCliente("Parou");
        break;
    case 3:
        logEventoCliente("Conectou-se ao servidor");
        break;
    case 4:
        logEventoCliente("Enviou um jogo ao servidor");
        break;
    case 5:
        logEventoCliente("Recebeu uma resposta do servidor");
        break;
    case 6:
        logEventoCliente("Desconectou-se do servidor");
        break;
    default:
        logEventoCliente("Evento desconhecido");
        break;
    }
}