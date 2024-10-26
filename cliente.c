#include "./headers/cliente.h"
#define CONFIGFILE "./clienteConfigs/cliente.conf"
// Aceita cliente1.conf até clienteN.conf

int idCliente = 0;

#define LINE_SIZE 16

// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
struct ClienteConfig clienteConfig;
struct JogoAtual jogoAtual;

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
    pthread_mutex_lock(&mutexClienteLog);
    clienteConfig.idCliente = idCliente + 1;
    pthread_mutex_unlock(&mutexClienteLog);

    return;
}

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

void lerJogo(int sock, char buffer[])
{
    // Read data from server
    int valread = read(sock, buffer, BUF_SIZE);
    buffer[valread] = '\0'; // Null-terminate the string

    // Parse the received JSON
    cJSON *jsonReceived = cJSON_Parse(buffer);
    if (jsonReceived == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        printf("Error parsing JSON: %s\n", error_ptr);
        exit(EXIT_FAILURE);
    }

    cJSON *idJogoItem = cJSON_GetObjectItem(jsonReceived, "idJogo");
    cJSON *TabuleiroJogoItem = cJSON_GetObjectItem(jsonReceived, "jogo");
    if (idJogoItem == NULL || TabuleiroJogoItem == NULL)
    {
        printf("idJogo not found in JSON\n");
        cJSON_Delete(jsonReceived); // Free JSON object
        exit(EXIT_FAILURE);
    }
    jogoAtual.idJogo = idJogoItem->valueint;
    strcpy(jogoAtual.jogo, TabuleiroJogoItem->valuestring);

    // Cleanup
    cJSON_Delete(jsonReceived);
}

void enviarJogo(int *sock)
{
    // Cria o objeto JSON para enviar o jogo
    cJSON *jsonEnvio = cJSON_CreateObject();
    if (jsonEnvio == NULL)
    {
        perror("Erro ao criar JSON");
        exit(EXIT_FAILURE);
    }

    // Adiciona os dados do jogo ao JSON
    cJSON_AddNumberToObject(jsonEnvio, "idJogo", jogoAtual.idJogo);
    cJSON_AddStringToObject(jsonEnvio, "jogo", jogoAtual.jogo);

    // Converte o JSON para uma string
    char *jsonString = cJSON_PrintUnformatted(jsonEnvio);
    if (jsonString == NULL)
    {
        perror("Erro ao converter JSON para string");
        cJSON_Delete(jsonEnvio); // Libera o JSON
        exit(EXIT_FAILURE);
    }

    // Envia o JSON para o servidor
    ssize_t bytesEnviados = send(*sock, jsonString, strlen(jsonString), 0);
    if (bytesEnviados < 0)
    {
        perror("Erro ao enviar jogo para o servidor");
        free(jsonString);        // Libera a string JSON
        cJSON_Delete(jsonEnvio); // Libera o JSON
        exit(EXIT_FAILURE);
    }

    printf("Jogo enviado com sucesso para o servidor.\n");

    // Limpeza
    free(jsonString);        // Libera a string JSON
    cJSON_Delete(jsonEnvio); // Libera o JSON
}

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
void logQueEventoCliente(int numero)
{
    switch (numero)
    {
    case 1:
        logEventoCliente("Cliente id: iniciou");
        break;
    case 2:
        logEventoCliente("Cliente id: parou");
        break;
    case 3:
        logEventoCliente("Cliente id: conectou-se ao servidor");
        break;
    case 4:
        logEventoCliente("Cliente id: enviou uma mensagem ao servidor");
        break;
    case 5:
        logEventoCliente("Cliente id: recebeu uma resposta do servidor");
        break;
    case 6:
        logEventoCliente("Cliente id: desconectou-se do servidor");
        break;
    default:
        logEventoCliente("Evento desconhecido");
        break;
    }
}
void enviarInfoCliente(int *sock)
{
    // Create a JSON object to send
    cJSON *jsonEnvio = cJSON_CreateObject();
    if (jsonEnvio == NULL)
    {
        perror("Erro ao criar JSON");
        exit(EXIT_FAILURE);
    }

    // Add client config data to the JSON
    cJSON_AddNumberToObject(jsonEnvio, "idCliente", clienteConfig.idCliente);
    cJSON_AddStringToObject(jsonEnvio, "tipoJogo", clienteConfig.tipoJogo);
    cJSON_AddStringToObject(jsonEnvio, "metodoResolucao", clienteConfig.metodoResolucao);

    // Convert the JSON object to a string
    char *jsonString = cJSON_PrintUnformatted(jsonEnvio);
    if (jsonString == NULL)
    {
        perror("Erro ao converter JSON para string");
        cJSON_Delete(jsonEnvio);
        exit(EXIT_FAILURE);
    }

    // Send the JSON string to the server
    ssize_t bytesEnviados = send(*sock, jsonString, strlen(jsonString), 0);
    if (bytesEnviados < 0)
    {
        perror("Erro ao enviar info para o servidor");
        free(jsonString);
        cJSON_Delete(jsonEnvio);
        exit(EXIT_FAILURE);
    }

    printf("Informações do cliente enviadas com sucesso para o servidor.\n");

    // Clean up
    free(jsonString);        // Free the JSON string
    cJSON_Delete(jsonEnvio); // Delete the JSON object
}
int main(int argc, char **argv)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};

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
    printf("TipoJogo:%s\nMetodo:%s\nID:%d\nIP:%s\nPorta:%d\n", clienteConfig.tipoJogo, clienteConfig.metodoResolucao, clienteConfig.idCliente, clienteConfig.ipServidor, clienteConfig.portaServidor);
    logEventoCliente("Cliente iniciado");
    ligacaoSocketC_S(&sock, serv_addr);
    enviarInfoCliente(&sock);
    lerJogo(sock, buffer);
    imprimirTabuleiro(jogoAtual.jogo);
    // enviarJogo(&sock);

    return 0;
}