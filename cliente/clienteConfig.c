#include "../headers/cliente.h"
#define CONFIGFILE "./clienteConfigs/cliente.conf"

pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
int globalIdCounter = 0;
struct Cliente cliente;

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
        strcpy(cliente.tipoJogo, resultado);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        char *resultado = strtok(buffer, "\n");
        strcpy(cliente.metodoResolucao, resultado);
    }
    // Leitura do IP do servidor (segunda linha)
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        char *resultado = strtok(buffer, "\n");
        strcpy(cliente.ipServidor, resultado);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        cliente.portaServidor = atoi(buffer);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        cliente.numeroLinhas = atoi(buffer);
    }
    if (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        cliente.numeroColunas = atoi(buffer);
    }

    contadorConfigs++; // Contar o número de configurações lidas

    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(EXIT_FAILURE);
    }
    char *vazio = "";
    if (!strcmp(cliente.ipServidor, vazio) ||
        !cliente.portaServidor ||
        !strcmp(cliente.tipoJogo, vazio) ||
        !strcmp(cliente.metodoResolucao, vazio))
    {
        printf("Algum campo da config nao preenchido\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&mutexClienteID);
    globalIdCounter++;
    printf("ID do cliente: %d\n", globalIdCounter);
    cliente.idCliente = globalIdCounter;
    pthread_mutex_unlock(&mutexClienteID);

    return;
}