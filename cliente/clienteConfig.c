#include "../headers/cliente.h"
#define CONFIGFILE "./clienteConfigs/cliente.conf"

pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;

int idCliente = 0;

struct ClienteConfig clienteConfig;

void carregarConfigCliente(char *nomeFicheiro)
{
    FILE *config = abrirFicheiro(nomeFicheiro);

    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    // esta para 1 cliente
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        // Leitura do tipoJogo (primeira linha)
        char *resultado = strtok(buffer, "\n");
        strcpy(clienteConfig.tipoJogo, resultado);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        char *resultado = strtok(buffer, "\n");
        strcpy(clienteConfig.metodoResolucao, resultado);
    }
    // Leitura do IP do servidor (segunda linha)
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        char *resultado = strtok(buffer, "\n");
        strcpy(clienteConfig.ipServidor, resultado);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        clienteConfig.portaServidor = atoi(buffer);
    }
    contadorConfigs++; // Contar o número de configurações lidas

    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(EXIT_FAILURE);
    }
    char *vazio = "";
    if (!strcmp(clienteConfig.ipServidor, vazio) ||
        !clienteConfig.portaServidor ||
        !strcmp(clienteConfig.tipoJogo, vazio) ||
        !strcmp(clienteConfig.metodoResolucao, vazio))
    {
        printf("Algum campo da config nao preenchido\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&mutexClienteID);
    idCliente += 1;
    clienteConfig.idCliente = idCliente;
    pthread_mutex_unlock(&mutexClienteID);

    return;
}