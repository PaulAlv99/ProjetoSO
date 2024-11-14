#include "../headers/servidor.h"

// structs
struct Jogo jogosEsolucoes[NUM_JOGOS];
// globais
static int idCliente = 0;
// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNTentativas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforoAguardaResposta;

// so tem o ficheiro da localizacao dos jogos e solucoes que neste caso e apenas 1 ficheiro
void carregarConfigServidor(char *nomeFicheiro, struct ServidorConfig *serverConfig)
{
    FILE *config = abrirFicheiroRead(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    // ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strcpy(serverConfig->ficheiroJogosESolucoesCaminho, strtok(buffer, "\n"));
        // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        serverConfig->porta = atoi(strtok(buffer, "\n"));
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
    char *ficheiroLogs = "logs/LogServidor.txt";
    pthread_mutex_lock(&mutexServidorLog);
    FILE *file = fopen(ficheiroLogs, "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexServidorLog);
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fecharFicheiro(file);
    pthread_mutex_unlock(&mutexServidorLog);
}

void logQueEventoServidor(int numero, int clienteID)
{
    char *mensagem = malloc(BUF_SIZE * sizeof(char));
    if (mensagem == NULL)
    {
        perror("Erro ao alocar memoria para mensagem");
        exit(1);
    }
    // case 1 e 2 mesmo que nao usem memoria é necessário libertar
    switch (numero)
    {
    case 1:
        logEventoServidor("Servidor comecou");
        free(mensagem);
        break;
    case 2:
        logEventoServidor("Servidor parou");
        free(mensagem);
        break;
    case 3:
        sprintf(mensagem, "Cliente-%d conectado", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 4:
        sprintf(mensagem, "Servidor enviou um jogo para o cliente-%d", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 5:
        sprintf(mensagem, "Servidor recebeu uma solucao do cliente-%d", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 6:
        sprintf(mensagem, "Servidor enviou uma solucao para o cliente-%d", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 7:
        sprintf(mensagem, "Cliente-%d desconectado", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 8:
        sprintf(mensagem, "Cliente-%d resolveu o jogo", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    default:
        logEventoServidor("Evento desconhecido");
        free(mensagem);
        break;
    }
}

// atualiza os valoresCorretos da Ultima Tentativa
char *atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas)
{
    // Aloca dinamicamente espaço para logClienteFinal
    char *logClienteFinal = malloc(2 * BUF_SIZE * sizeof(char));
    if (logClienteFinal == NULL)
    {
        perror("Erro ao alocar memoria para logClienteFinal");
        exit(1);
    }

    char logCliente[BUF_SIZE] = "\n";
    char Tentativas[100];
    sprintf(Tentativas, "Tentativa n: %d\n", *nTentativas);

    // Retornar tentativas para escrever no log do cliente
    for (int i = 0; i < strlen(tentativaAtual); i++)
    {
        if (valoresCorretos[i] == '0' && tentativaAtual[i] != '0')
        {
            if (tentativaAtual[i] == solucao[i])
            {
                valoresCorretos[i] = tentativaAtual[i];
                char message[BUF_SIZE] = "";
                sprintf(message, "Valor correto(%c), na posição %d da String \n", tentativaAtual[i], i + 1);
                strcat(logCliente, message);
            }
            else
            {
                char message[BUF_SIZE] = "";
                sprintf(message, "Valor incorreto(%c), na posição %d da String \n", tentativaAtual[i], i + 1);
                strcat(logCliente, message);
            }
        }
    }

    pthread_mutex_lock(&mutexNTentativas);
    *nTentativas = *nTentativas + 1;
    pthread_mutex_unlock(&mutexNTentativas);

    snprintf(logClienteFinal, BUF_SIZE, "%s", logCliente);
    return logClienteFinal;
}

// Atualiza o booleano Resolvido se o jogo tiver sido resolvido
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido)
{
    for (int i = 0; i < strlen(valoresCorretos); i++)
    {
        if (valoresCorretos[i] != solucao[i])
        {
            return false;
        }
    }
    return true;
}

void carregarFicheiroJogosSolucoes(char *nomeFicheiro)
{
    FILE *config = abrirFicheiroRead(nomeFicheiro);

    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    // le todos os jogos
    while (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        // Leitura do idJogo (primeira linha)
        char *IDJogo = strtok(buffer, "\n");
        jogosEsolucoes[contadorConfigs].idJogo = atoi(IDJogo);
        // Leitura jogo (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL)
        {
            char *Jogo = strtok(buffer, "\n");
            strcpy(jogosEsolucoes[contadorConfigs].jogo, Jogo);
            // Leitura solucao (terceira linha)
            if (fgets(buffer, BUF_SIZE, config) != NULL)
            {
                char *Solucao = strtok(buffer, "\n");
                strcpy(jogosEsolucoes[contadorConfigs].solucao, Solucao);
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

struct ServidorConfig construtorServer(int dominio, int servico, int protocolo, __u_long interface, int porta, int backlog, char *ficheiroJogosESolucoesCaminho)
{
    struct ServidorConfig servidor = {0};
    servidor.dominio = dominio;
    servidor.servico = servico;
    servidor.protocolo = protocolo;
    servidor.interface = interface;
    servidor.porta = porta;
    servidor.backlog = backlog;
    strcpy(servidor.ficheiroJogosESolucoesCaminho, ficheiroJogosESolucoesCaminho);
    return servidor;
}

void iniciarServidorSocket(struct ServidorConfig *server)
{
    int socketServidor = socket(server->dominio, server->servico, server->protocolo);
    if (socketServidor == -1)
    {
        perror("Erro ao criar socket");
        exit(1);
    }
    struct sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = server->dominio;
    enderecoServidor.sin_port = htons(server->porta);
    enderecoServidor.sin_addr.s_addr = server->interface;
    int bindResult = bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor));
    if (bindResult == -1)
    {
        perror("Erro ao fazer bind");
        exit(1);
    }
    int listenResult = listen(socketServidor, server->backlog);
    if (listenResult == -1)
    {
        perror("Erro ao fazer listen");
        exit(1);
    }
    printf("==== Servidor Iniciado ====\n");
    printf("====== Porta: %d =======\n\n", server->porta);

    while (1)
    {
        // sempre a aceitar novas conexoes
        struct sockaddr_in enderecoCliente;
        int tamanhoEndereco = sizeof(enderecoCliente);
        int socketCliente = accept(socketServidor, (struct sockaddr *)&enderecoCliente, (socklen_t *)&tamanhoEndereco);
        // aceitar conexao se falhar volta para o inicio while e tenta aceitar outras conexoes
        if (socketCliente == -1)
        {
            perror("Erro ao aceitar conexão");
            continue;
        }
        char temp[BUF_SIZE] = {0};
        pthread_mutex_lock(&mutexClienteID);
        idCliente++;
        sprintf(temp, "%d|", idCliente);
        pthread_mutex_unlock(&mutexClienteID);

        // cria filho para tratar do cliente e fecha o pai
        if (fork() == 0)
        {
            // Processo filho
            struct ClienteConfig clienteConfig = {0};
            close(socketServidor);

            // enviar para o cliente o seu ID
            write(socketCliente, temp, BUF_SIZE);
            char *msgLog = malloc(2 * BUF_SIZE * sizeof(char));
            if (msgLog == NULL)
            {
                perror("Erro ao alocar memoria");
                exit(1);
            }
            logQueEventoServidor(3, idCliente);
            printf("%s\n", msgLog);

            char buffer[BUF_SIZE] = {0};
            // rand time dá sempre mesma seed se chegarem ao mesmo tempo
            srand(getpid());
            int nJogo = rand() % NUM_JOGOS;
            char *jogoADar = jogosEsolucoes[nJogo].jogo;
            free(msgLog);
            receberMensagemETratarServer(buffer, socketCliente, clienteConfig, nJogo, jogoADar);
            close(socketCliente);
            exit(0);
        }
        else
        {
            // Processo pai
            close(socketCliente);
        }
    }
    close(socketServidor);
}

void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, int nJogo, char *jogoADar)
{
    while (recv(socketCliente, buffer, BUF_SIZE, 0) > 0)
    {  
        char bufferFinal[BUF_SIZE] = {0};
        sprintf(bufferFinal, "Mensagem recebida: %s\n", buffer);
        logEventoServidor(bufferFinal);
        char *idCliente = strtok(buffer, "|");
        char *tipoJogo = strtok(NULL, "|");
        char *tipoResolucao = strtok(NULL, "|");
        char *temJogo = strtok(NULL, "|");
        char *idJogo = strtok(NULL, "|");
        char *jogo = strtok(NULL, "|");
        char *valoresCorretos = strtok(NULL, "|");
        char *tempoInicio = strtok(NULL, "|");
        char *tempoFinal = strtok(NULL, "|");
        char *resolvido = strtok(NULL, "|");
        char *numeroTentativas = strtok(NULL, "|");
        if (strcmp(temJogo, "SEM_JOGO") == 0)
        {
            if ((idCliente || tipoJogo || tipoResolucao || temJogo || idJogo || jogo || valoresCorretos || tempoInicio || tempoFinal || resolvido || numeroTentativas) == NULL)
            {
                printf("Erro: Mensagem recebida com formato inválido\n");
                break;
            }

            // sem_wait(&semaforoAguardaResposta);

            strcpy(clienteConfig.tipoJogo, tipoJogo);
            strcpy(clienteConfig.tipoResolucao, tipoResolucao);
            strcpy(clienteConfig.TemJogo, "COM_JOGO");
            strcpy(clienteConfig.jogoAtual.jogo, jogoADar);
            strcpy(clienteConfig.jogoAtual.valoresCorretos, jogoADar);
            strcpy(clienteConfig.jogoAtual.tempoInicio, getTempoHoraMinutoSegundo());
            strcpy(clienteConfig.jogoAtual.tempoFinal, tempoFinal);
            clienteConfig.jogoAtual.resolvido = atoi(resolvido);
            clienteConfig.jogoAtual.numeroTentativas = atoi(numeroTentativas);
            clienteConfig.jogoAtual.idJogo = nJogo;
            clienteConfig.idCliente = atoi(idCliente);

            memset(buffer, 0, BUF_SIZE);
            sprintf(buffer, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d",
                    clienteConfig.idCliente,
                    clienteConfig.tipoJogo,
                    clienteConfig.tipoResolucao,
                    clienteConfig.TemJogo,
                    clienteConfig.jogoAtual.idJogo,
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    clienteConfig.jogoAtual.tempoInicio,
                    clienteConfig.jogoAtual.tempoFinal,
                    clienteConfig.jogoAtual.resolvido,
                    clienteConfig.jogoAtual.numeroTentativas);

            write(socketCliente, buffer, BUF_SIZE);
            // sem_post(&semaforoAguardaResposta);
            char bufferEnviarFinal[BUF_SIZE] = {0};
            sprintf(bufferEnviarFinal, "Mensagem enviada: %s\n", buffer);
            logQueEventoServidor(4, clienteConfig.idCliente);
            logEventoServidor(bufferEnviarFinal);
            memset(buffer, 0, BUF_SIZE);
        }

        if (strcmp(temJogo, "COM_JOGO") == 0)
        {
            if ((idCliente || tipoJogo || tipoResolucao || temJogo || idJogo || jogo || valoresCorretos || tempoInicio || tempoFinal || resolvido || numeroTentativas) == NULL)
            {
                printf("Erro: Mensagem recebida com formato inválido\n");
                break;
            }

            clienteConfig.idCliente = atoi(idCliente);
            strcpy(clienteConfig.TemJogo, temJogo);
            strcpy(clienteConfig.tipoJogo, tipoJogo);
            strcpy(clienteConfig.tipoResolucao, tipoResolucao);
            clienteConfig.jogoAtual.idJogo = nJogo;
            strcpy(clienteConfig.jogoAtual.jogo, jogo);
            strcpy(clienteConfig.jogoAtual.valoresCorretos, valoresCorretos);
            strcpy(clienteConfig.jogoAtual.tempoInicio, tempoInicio);
            strcpy(clienteConfig.jogoAtual.tempoFinal, tempoFinal);
            clienteConfig.jogoAtual.resolvido = atoi(resolvido);
            clienteConfig.jogoAtual.numeroTentativas = atoi(numeroTentativas);

            logQueEventoServidor(5, clienteConfig.idCliente);

            char *logClienteEnviar;
            if (strcmp(clienteConfig.tipoResolucao, "COMPLET") == 0)
            {
                logClienteEnviar = atualizaValoresCorretos(
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    jogosEsolucoes[clienteConfig.jogoAtual.idJogo].solucao,
                    &clienteConfig.jogoAtual.numeroTentativas);
                if (logClienteEnviar == NULL)
                {
                    logClienteEnviar = "";
                }
            }
            if (strcmp(clienteConfig.tipoResolucao, "PARCIAL") == 0)
            {
                logClienteEnviar = atualizaValoresCorretos(
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    jogosEsolucoes[clienteConfig.jogoAtual.idJogo].solucao,
                    &clienteConfig.jogoAtual.numeroTentativas);
                if (logClienteEnviar == NULL)
                {
                    logClienteEnviar = "";
                }
            }

            if (verificaResolvido(
                    clienteConfig.jogoAtual.valoresCorretos,
                    jogosEsolucoes[clienteConfig.jogoAtual.idJogo].solucao,
                    clienteConfig.jogoAtual.resolvido))
            {
                clienteConfig.jogoAtual.resolvido = 1;
                logQueEventoServidor(8, clienteConfig.idCliente);
            }

            memset(buffer, 0, BUF_SIZE);
            sprintf(buffer, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d|%s",
                    clienteConfig.idCliente,
                    clienteConfig.tipoJogo,
                    clienteConfig.tipoResolucao,
                    clienteConfig.TemJogo,
                    clienteConfig.jogoAtual.idJogo,
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    clienteConfig.jogoAtual.tempoInicio,
                    clienteConfig.jogoAtual.tempoFinal,
                    clienteConfig.jogoAtual.resolvido,
                    clienteConfig.jogoAtual.numeroTentativas,
                    logClienteEnviar);

            write(socketCliente, buffer, BUF_SIZE);
            free(logClienteEnviar);
            // sem_post(&semaforoAguardaResposta);
            char bufferFinal[BUF_SIZE] = {0};
            sprintf(bufferFinal, "Mensagem enviada: %s\n", buffer);
            logQueEventoServidor(6, clienteConfig.idCliente);
            logEventoServidor(bufferFinal);
            memset(buffer, 0, BUF_SIZE);
        }
    }
    char *temp;
    sprintf(temp, "Cliente-%d desconectado", clienteConfig.idCliente);
    printf("%s\n", temp);
}

int main(int argc, char **argv)
{
    struct ServidorConfig serverConfig = {0};

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
    carregarConfigServidor(argv[1], &serverConfig);
    // printf("Caminho jogos e solucoes: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho);
    serverConfig = construtorServer(AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 1, serverConfig.ficheiroJogosESolucoesCaminho);
    // server posso considerar id 0 que nao vou usar para nada
    logQueEventoServidor(1, 0);
    iniciarServidorSocket(&serverConfig);
    return 0;
}