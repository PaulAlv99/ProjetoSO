#include "../headers/servidor.h"
#define CONFIGFILE "./servidorConfig/servidor.conf"
// structs
struct ServidorConfig serverConfig;
struct Jogo jogosEsolucoes[NUM_JOGOS];

// void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[])
// {
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
//         {
//             char numero = tentativaAtual[i];
//             int numeroInt = (int)(numero);
//             int novoNumero = numeroInt + 1;
//             char novoNumeroChar = (char)(novoNumero);
//             tentativaAtual[i] = novoNumeroChar;
//         }
//         else if (tentativaAtual[i] == '0')
//         {
//             tentativaAtual[i] = '1';
//         }
//     }
// }

// // atualiza os valoresCorretos da Ultima Tentativa
// void atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas)
// {
//     char Tentativas[100];
//     sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
//     logEventoCliente(Tentativas);
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if (valoresCorretos[i] == '0')
//         {
//             if (tentativaAtual[i] == solucao[i])
//             {
//                 valoresCorretos[i] = tentativaAtual[i];
//                 char message[1024];
//                 sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);

//                 // printf("%d \n", valoresCorretos);
//             }
//             else
//             {
//                 char message[1024];
//                 sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//             }
//         }
//     }
// }

// // // Atualiza o booleano Resolvido se o jogo tiver sido resolvido
// bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido)
// {
//     for (int i = 0; i < strlen(valoresCorretos); i++)
//     {
//         if (valoresCorretos[i] != solucao[i])
//         {
//             return false;
//         }
//     }
//     return true;
// }

// // ResolveJogo
// void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){

//         nTentativas = nTentativas + 1;
//         tentarSolucaoCompleta(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosCompletos(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }

// // Solucao parcial (Tentar um valor por vez)
// void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[])
// {
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
//         {
//             char numero = tentativaAtual[i];
//             int numeroInt = (int)(numero);
//             int novoNumero = numeroInt + 1;
//             char novoNumeroChar = (char)(novoNumero);
//             tentativaAtual[i] = novoNumeroChar;
//             break;
//         }
//         else if (tentativaAtual[i] == '0')
//         {
//             tentativaAtual[i] = '1';
//             break;
//         }
//     }
// }

// void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas)
// {
//     char Tentativas[100];
//     sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
//     logEventoCliente(Tentativas);
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if (valoresCorretos[i] == '0')
//         {
//             if (tentativaAtual[i] == solucao[i])
//             {
//                 valoresCorretos[i] = tentativaAtual[i];
//                 char message[1024];
//                 sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//                 // printf("%d \n", valoresCorretos);
//             }
//             else
//             {
//                 char message[1024];
//                 sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//             }
//         }
//     }
// }
// //ResolveJogo
// void resolverJogoParcial(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){
//         nTentativas = nTentativas + 1;
//         tentarSolucaoParcial(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosParcial(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }

void handleClient(int client_socket)
{
    char tentativa[BUF_SIZE];

    // Continue receiving guesses from the client
    while (1)
    {
        int bytes_received = recv(client_socket, tentativa, BUF_SIZE, 0);
        struct ClienteInfo info;
        if (bytes_received <= 0)
        {
            printf("Client-disconnected\n");
            break; // Client disconnected or error
        }

        tentativa[bytes_received] = '\0'; // Null-terminate the received guess
        printf("Received guess: %s\n", tentativa);
    }
}
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
    address.sin_family = AF_INET;          // Define a família de endereços como IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Aceita conexões de qualquer endereço IP
    address.sin_port = htons(SERVER_PORT); // Define a porta do servidor

    // Associa o socket à porta
    if (bind(*server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Falha ao associar socket"); // Mensagem de erro se a associação falhar
        close(*server_fd);                  // Fecha o socket
        exit(EXIT_FAILURE);
    }

    // Escuta por conexões de entrada
    if (listen(*server_fd, NUM_CLIENTES) < 0)
    {
        perror("Falha ao escutar"); // Mensagem de erro se a escuta falhar
        close(*server_fd);          // Fecha o socket
        exit(EXIT_FAILURE);
    }
    printf("Servidor na porta %d\n", SERVER_PORT); // Mensagem indicando que o servidor está escutando

    // Aceita uma conexão de entrada
    while ((*new_socket = accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        handleClient(*new_socket);
        close(*new_socket);
    }

    close(*server_fd);
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
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho);
    ligacaoSocketS_C(&server_fd, &new_socket, address);
    return 0;
}