#include "../headers/cliente.h"
#define CONFIGFILE "./clienteConfigs/cliente.conf"

#define LINE_SIZE 16

struct JogoAtual jogoAtual;
struct Cliente cliente;

void imprimirTabuleiro(char *jogo)
{
    for (int i = 0; i < cliente.numeroLinhas; i++)
    {
        if (i % 3 == 0 && i != 0)
        {
            printf("---------------------\n"); // Linha separadora horizontal
        }
        for (int j = 0; j < cliente.numeroLinhas; j++)
        {
            if (j % 3 == 0 && j != 0)
            {
                printf(" | "); // Separador vertical
            }
            printf("%c ", jogo[i * cliente.numeroLinhas + j]); // Imprime espaço para 0
        }
        printf("\n");
    }
}

// Função para carregar as configurações do cliente
// apenas vai ler id e ip se tiver mais alguma coisa ignora

void ligacaoSocketC_S(int *sock, struct sockaddr_in serv_addr)
{
    // Criar socket
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(cliente.portaServidor);

    // text para binary
    if (inet_pton(AF_INET, cliente.ipServidor, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported\n");
        exit(EXIT_FAILURE);
    }

    // Conectar ao servidor
    int esperaServidor = 1;
    while (connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        if (esperaServidor)
        {
            printf("Á espera do servidor\n");
            esperaServidor = 0;
        }
    }
    printf("Servidor iniciou\n");
}
int config_loaded = 0;

void *client_thread(void *arg)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if (!config_loaded)
    {
        carregarConfigCliente(CONFIGFILE);
        logQueEventoCliente(1);
        config_loaded = 1; // Mark config as loaded
    }

    ligacaoSocketC_S(&sock, serv_addr);
    char *temp = mandarDadosCliente_Jogo();
    send(sock, temp, strlen(temp), 0);
    printf("Data sent to the server.\n");

    close(sock);
    pthread_exit(NULL);
}
char *mandarDadosCliente_Jogo()
{
    static char temp[20000]; // Use static array to retain data
    // Now it's safe to create the string
    if (cliente.jogoAtual != NULL)
    {
        snprintf(temp, sizeof(temp), "%d|%s|%s|%d|%s|%d",
                 cliente.idCliente,
                 cliente.tipoJogo,
                 cliente.metodoResolucao,
                 cliente.jogoAtual->idJogo,
                 cliente.jogoAtual->jogo,
                 cliente.jogoAtual->numeroTentativas);
    }
    snprintf(temp, sizeof(temp), "%d|%s|%s",
             cliente.idCliente,
             cliente.tipoJogo,
             cliente.metodoResolucao);
    return temp; // Return the constructed string
}
int main(int argc, char **argv)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Verifica se foi fornecido um nome de arquivo
    if (argc < 2)
    {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Valida o nome do arquivo passado como argumento
    if (!validarNomeFile(argv[1], CONFIGFILE))
    {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }
    // Array of thread IDs
    pthread_t clients[2];

    // Create threads to simulate clients
    for (int i = 0; i < 2; i++)
    {
        if (pthread_create(&clients[i], NULL, client_thread, NULL) != 0)
        {
            printf("Error creating thread %d\n", i + 1);
            return 1;
        }
        printf("Thread %d started.\n", i + 1);
    }

    // Wait for all threads to complete
    for (int i = 0; i < 2; i++)
    {
        pthread_join(clients[i], NULL);
    }

    printf("All client threads finished.\n");

    return 0;
}