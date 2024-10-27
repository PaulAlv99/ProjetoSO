#include "../headers/servidor.h"
#define CONFIGFILE "./servidorConfig/servidor.conf"
// structs
struct Servidor servidor;
struct Jogo jogosEsolucoes[5];

// Função para estabelecer a ligação do servidor com os clientes
void ligacaoSocketS_C(int *server_fd, int *new_socket, struct sockaddr_in address)
{
    int opt = 1;                   // Opção para o socket
    int addrlen = sizeof(address); // Tamanho da estrutura de endereço

    // Cria o descritor de socket
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Falha ao criar socket"); // Mensagem de erro se a criação do socket falhar
        exit(EXIT_FAILURE);
    }

    // Configura SO_REUSEPORT
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Falha ao configurar SO_REUSEPORT"); // Mensagem de erro se a configuração falhar
        close(*server_fd);                          // Fecha o socket
        exit(EXIT_FAILURE);
    }

    // Configura SO_KEEPALIVE
    if (setsockopt(*server_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
    {
        perror("Falha ao configurar SO_KEEPALIVE"); // Mensagem de erro se a configuração falhar
        close(*server_fd);                          // Fecha o socket
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;                     // Define a família de endereços como IPv4
    address.sin_addr.s_addr = INADDR_ANY;             // Aceita conexões de qualquer endereço IP
    address.sin_port = htons(servidor.portaServidor); // Define a porta do servidor

    // Associa o socket à porta
    if (bind(*server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Falha ao associar socket"); // Mensagem de erro se a associação falhar
        close(*server_fd);                  // Fecha o socket
        exit(EXIT_FAILURE);
    }

    // Escuta por conexões de entrada
    if (listen(*server_fd, servidor.numeroConexoes) < 0)
    {
        perror("Falha ao escutar"); // Mensagem de erro se a escuta falhar
        close(*server_fd);          // Fecha o socket
        exit(EXIT_FAILURE);
    }
    printf("Servidor na porta %d\n", servidor.portaServidor); // Mensagem indicando que o servidor está escutando
    // Aceita uma nova conexão
    while ((*new_socket = accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        lerMensagem(server_fd, new_socket);
    }

    close(*new_socket);
    close(*server_fd);
}
void lerMensagem(int *server_fd, int *new_socket)
{
    char buffer[BUF_SIZE];
    int size;

    while ((size = read(*new_socket, buffer, BUF_SIZE - 1)) > 0)
    {
        buffer[size] = '\0'; // Null-terminate the buffer
        char *response = strtok(buffer, "|");
        while (response != NULL)
        {
            printf("Received: %s\n", response);
            logEventoServidor(response);
            response = strtok(NULL, "|");
        }
    }

    if (size < 0)
    {
        perror("Failed to read from socket");
    }
    else if (size == 0)
    {
        printf("Client disconnected\n");
    }

    close(*new_socket); // Close the socket after reading is done
}
int main(int argc, char **argv)
{
    int server_fd, new_socket;
    struct sockaddr_in address;

    if (argc < 2)
    {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

    if (strcmp(argv[1], CONFIGFILE) != 0)
    {
        printf("Nome do ficheiro incorreto\n");
        return 1;
    }

    carregarConfigServidor(argv[1]);
    carregarFicheiroJogosSolucoes(servidor.ficheiroJogosESolucoesCaminho);
    ligacaoSocketS_C(&server_fd, &new_socket, address);
    return 0;
}