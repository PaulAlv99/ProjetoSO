#include "./headers/servidor.h"
#define CONFIGFILE "./servidorConfig/servidor.conf"
#include "./headers/cliente.h"
// structs
struct ServidorConfig serverConfig;
struct Jogo jogosEsolucoes[NUM_JOGOS];
// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;

// so tem o ficheiro da localizacao dos jogos e solucoes que neste caso e apenas 1 ficheiro
void carregarConfigServidor(char *nomeFicheiro)
{
    FILE *config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    // ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strcpy(serverConfig.ficheiroJogosESolucoesCaminho, buffer);
        // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    }
    contadorConfigs++;
    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(1);
    }

    return;
}

void logEventoServidor(const char *message)
{

    // modo append
    char *ficheiroLogs = "servidorLogs/LogServidor.txt";
    pthread_mutex_lock(&mutexServidorLog);
    FILE *file = fopen(ficheiroLogs, "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexServidorLog);
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
    pthread_mutex_unlock(&mutexServidorLog);
}
void logQueEventoServidor(int numero)
{
    switch (numero)
    {
    case 1:
        logEventoServidor("Servidor começou");
        break;
    case 2:
        logEventoServidor("Socket começou");
        break;
    default:
        logEventoServidor("Evento desconhecido");
        break;
    }
}
// void verificarLinha(int linha, char* jogo, char* solucao) {
//     //Verifica uma linha do Sudoku (9 posições)
//     for (int i = 0; i < NUM_LINHAS; i++) {
//         int pos = linha * 9 + i;
//         //Verifica se a célula foi preenchida no jogo
//         if (jogo[pos] != solucao[pos]) {
//             //Imprime posição na forma posx-y comecando em 1
//             int coluna = (pos % 9) + 1;
//             printf("Posição errada: pos%d-%d\n", linha + 1, coluna);
//             //começa a contar do 0. pode ser guardado numa estrutura de dados para
//             //posteriormente mandar para o cliente
//             printf("Posicao na string: %d\n",pos);
//         }
//         else{
//             //Imprime posição na forma posx-y comecando em 1
//             int coluna = (pos % 9) + 1;
//             printf("Posição Correta: pos%d-%d\n", linha + 1, coluna);
//             //começa a contar do 0. pode ser guardado numa estrutura de dados para
//             //posteriormente mandar para o cliente
//             printf("Posicao na string: %d\n",pos);
//         }
//     }
// }

// void resolveJogo(char* jogo, char* solucao) {
//     //Verifica linha por linha (0 a 8, correspondente às 9 linhas)
//     for (int linha = 0; linha < 9; linha++) {
//         verificarLinha(linha, jogo, solucao);
//     }
// }
// Atualiza a tentativaAtual
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

// // Atualiza o booleano Resolvido se o jogo tiver sido resolvido
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
// // ResolveJogo void resolverJogoParcial(char jogo[], char solucao[], int nTentativas)
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

void carregarFicheiroJogosSolucoes(char *nomeFicheiro)
{
    FILE *config = abrirFicheiro(nomeFicheiro);

    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    while (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        // Leitura do idJogo (primeira linha)
        char *resultado = strtok(buffer, "\n");
        jogosEsolucoes[contadorConfigs].idJogo = atoi(resultado);
        // Leitura do IP do servidor (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL)
        {
            char *resultado = strtok(buffer, "\n");
            strcpy(jogosEsolucoes[contadorConfigs].jogo, resultado);
            if (fgets(buffer, BUF_SIZE, config) != NULL)
            {
                char *resultado = strtok(buffer, "\n");
                strcpy(jogosEsolucoes[contadorConfigs].solucao, resultado);
            }
        }

        contadorConfigs++; // Contar o número de configurações lidas
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(1);
    }
    // printf("Configs n:%d\n",contadorConfigs);
    return;
}

// Função para serializar a estrutura Jogo em formato JSON
char *JogoParaJSON(struct Jogo *jogo)
{
    // Cria um objeto JSON
    cJSON *jsonJogo = cJSON_CreateObject();
    if (jsonJogo == NULL)
    {
        return NULL; // Retorna NULL se a criação do objeto JSON falhar
    }

    // Adiciona propriedades ao objeto JSON
    cJSON_AddNumberToObject(jsonJogo, "idJogo", jogo->idJogo); // Adiciona o id do jogo
    cJSON_AddStringToObject(jsonJogo, "jogo", jogo->jogo);     // Adiciona o estado do jogo

    // Converte o objeto JSON em uma string
    char *jsonString = cJSON_Print(jsonJogo);
    cJSON_Delete(jsonJogo); // Libera a memória do objeto JSON criado

    return jsonString; // Retorna a string JSON
}

int jogoAleatorio()
{
    srand(time(NULL));
    return rand() % NUM_JOGOS;
}
char *serializarJogo()
{
    // Send jogo data to client
    int tempID = jogoAleatorio();
    if (jogosEsolucoes[tempID].idJogo != 0)
    { // Assuming idJogo 0 means it's empty
        char *jsonString = JogoParaJSON(&jogosEsolucoes[tempID]);
        return jsonString;
    }
    return NULL;
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

    // Configura o socket para permitir reutilização do endereço
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Falha ao configurar socket"); // Mensagem de erro se a configuração falhar
        close(*server_fd);                    // Fecha o socket
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
    printf("Servidor está escutando na porta %d\n", SERVER_PORT); // Mensagem indicando que o servidor está escutando

    // Aceita uma conexão de entrada
    if ((*new_socket = accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Falha ao aceitar conexão"); // Mensagem de erro se a aceitação falhar
        close(*server_fd);                  // Fecha o socket
        exit(EXIT_FAILURE);
    }
}

void enviarJogo(int new_socket)
{
    char *jsonString = serializarJogo();
    if (jsonString != NULL)
    {
        send(new_socket, jsonString, strlen(jsonString), 0);
        printf("Sent jogo to client: %s\n", jsonString);
        free(jsonString); // Free the JSON string after sending
    }
}
void receberInfoCliente(int *sock)
{
    // Buffer para armazenar os dados recebidos
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer)); // Limpa o buffer

    // Recebe dados do cliente
    ssize_t bytesRecebidos = recv(*sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRecebidos < 0)
    {
        perror("Erro ao receber informações do cliente");
        exit(EXIT_FAILURE);
    }
    // else if (bytesRecebidos == 0)
    // {
    //     printf("Conexão encerrada pelo cliente.\n");
    //     return;
    // }

    // Termina a string recebida com nulo
    buffer[bytesRecebidos] = '\0';

    // Imprime os dados recebidos
    printf("Dados recebidos do cliente: %s\n", buffer);

    // Analisa os dados JSON
    cJSON *jsonRecebido = cJSON_Parse(buffer);
    if (jsonRecebido == NULL)
    {
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        exit(EXIT_FAILURE);
    }

    // Extrai dados do objeto JSON (atualize as chaves conforme a estrutura do seu JSON)
    cJSON *idCliente = cJSON_GetObjectItem(jsonRecebido, "idCliente");
    cJSON *tipoJogo = cJSON_GetObjectItem(jsonRecebido, "tipoJogo");
    cJSON *metodoResolucao = cJSON_GetObjectItem(jsonRecebido, "metodoResolucao");

    if (cJSON_IsNumber(idCliente) && cJSON_IsString(tipoJogo) && cJSON_IsString(metodoResolucao))
    {
        printf("ID Cliente: %d\n", idCliente->valueint);
        printf("Tipo de Jogo: %s\n", tipoJogo->valuestring);
        printf("Método de Resolução: %s\n", metodoResolucao->valuestring);
    }
    else
    {
        fprintf(stderr, "Erro: Dados JSON não estão no formato esperado.\n");
    }

    // Limpeza
    cJSON_Delete(jsonRecebido); // Libera o objeto JSON
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
    logQueEventoServidor(1);
    ligacaoSocketS_C(&server_fd, &new_socket, address);
    enviarJogo(new_socket);
    receberInfoCliente(&new_socket);

    return 0;
}