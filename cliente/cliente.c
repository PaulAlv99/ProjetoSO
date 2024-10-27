#include "../headers/cliente.h"
#define CONFIGFILE "./clienteConfigs/cliente.conf"

#define LINE_SIZE 16

struct JogoAtual jogoAtual;
struct ClienteConfig clienteConfig;

void imprimirTabuleiro(char *jogo)
{
    for (int i = 0; i < NUM_LINHAS; i++)
    {
        if (i % 3 == 0 && i != 0)
        {
            printf("---------------------\n"); // Linha separadora horizontal
        }
        for (int j = 0; j < NUM_LINHAS; j++)
        {
            if (j % 3 == 0 && j != 0)
            {
                printf(" | "); // Separador vertical
            }
            printf("%c ", jogo[i * NUM_LINHAS + j]); // Imprime espaço para 0
        }
        printf("\n");
    }
}

// Função para carregar as configurações do cliente
// apenas vai ler id e ip se tiver mais alguma coisa ignora

void ligacaoSocketC_S(int *sock, struct sockaddr_in serv_addr)
{
    // Create socket
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(clienteConfig.portaServidor);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, clienteConfig.ipServidor, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported\n");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    int esperaServidor = 1;
    while (connect(*sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        if (esperaServidor)
        {
            printf("\nProblema de ligação\n");
            printf("Á espera do servidor\n");
            esperaServidor = 0;
        }
    }
    printf("Servidor iniciou\n");
}
void serializeClienteInfo(struct ClienteConfig info, char *output)
{
    snprintf(output, BUF_SIZE, "%d|%s|%s|%d|%s|%d",
             info.idCliente,
             info.tipoJogo,
             info.metodoResolucao,
             jogoAtual.idJogo,
             jogoAtual.jogo,
             jogoAtual.numeroTentativas);
}
int main(int argc, char **argv)
{
    char serializedData[BUF_SIZE];
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
    carregarConfigCliente(argv[1]);
    logQueEventoCliente(1);
    ligacaoSocketC_S(&sock, serv_addr);
    serializeClienteInfo(clienteConfig, &serializedData);
    send(sock, serializedData, strlen(serializedData), 0);
    printf("Data sent to the server.\n");
}