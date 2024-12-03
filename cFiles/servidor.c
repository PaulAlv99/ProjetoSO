#include "../headers/servidor.h"
// TODO CLIENTE sai da sala mas tem probalidade de voltar a entrar na fila-SINGLEPLAYER
// TODO GUARDAR INFO DE CADA LEADERBOARD DA SALA-SINGLEPLAYER
// TODO FAZER CADA SALA COM JOGO FIXO QUANDO INICIA-SINGLEPLAYER-FEITO CADA SALA TEM O JOGO COM O ID DO NUMERO DA SALA
// TODO QUANDO ELE TERMINA O JOGO O CLIENTE ENCERRA A COMUNICACAO. TENHO DE FAZER DE MODO A QUE NAO
// ENCERRE COMUNICACAO PARA O CLIENTE CONSEGUIR ENTRAR NA FILA OUTRA VEZ
//  structs
struct Jogo jogosEsolucoes[NUM_JOGOS];
struct filaClientesSinglePlayer *filaClientesSinglePlayer;

// globais
static int idCliente = 0;
static int idSala = 0;

// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNTentativas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIDSala = PTHREAD_MUTEX_INITIALIZER;

// semaforos
sem_t semaforoEntrarFila;

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

// TODO METER LOGS QUANDO ENTRA E SAI DAS SALAS
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
    servidor.sala = malloc(NUM_JOGOS * sizeof(struct SalaSinglePlayer));
    if (!servidor.sala)
    {
        perror("Erro ao alocar memória para salas");
        exit(1);
    }
    memset(servidor.sala, 0, NUM_JOGOS * sizeof(struct SalaSinglePlayer));
    return servidor;
}

void *criaClienteThread(void *arg)
{
    struct ThreadCliente *args = (struct ThreadCliente *)arg;
    int socketCliente = args->socketCliente;
    // struct ServidorConfig *server = args->server;
    int clientID = args->clienteId;

    struct ClienteConfig clienteConfig = {0};
    // char buffer[BUF_SIZE] = {0};

    char temp[BUF_SIZE] = {0};
    sprintf(temp, "%d|", clientID);

    writeSocket(socketCliente, temp, BUF_SIZE);
    logQueEventoServidor(3, clientID);

    // dar handle das mensagens
    receberMensagemETratarServer(temp, socketCliente, clienteConfig, *args->server);

    close(socketCliente);
    free(args);
    return NULL;
}

void iniciarServidorSocket(struct ServidorConfig *server)
{
    int socketServidor = socket(server->dominio, server->servico, server->protocolo);
    if (socketServidor == -1)
    {
        perror("Erro ao criar socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(1);
    }

    struct sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = server->dominio;
    enderecoServidor.sin_port = htons(server->porta);
    enderecoServidor.sin_addr.s_addr = server->interface;

    if (bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) == -1)
    {
        perror("Erro ao fazer bind");
        exit(1);
    }

    if (listen(socketServidor, server->backlog) == -1)
    {
        perror("Erro ao fazer listen");
        exit(1);
    }

    printf("==== Servidor Iniciado ====\n");
    printf("====== Porta: %d =======\n\n", server->porta);

    sem_init(&semaforoEntrarFila, 1, 1);
    iniciarSalasJogoSinglePlayer(server);
    while (1)
    {
        struct sockaddr_in enderecoCliente;
        int tamanhoEndereco = sizeof(enderecoCliente);
        int socketCliente = accept(socketServidor, (struct sockaddr *)&enderecoCliente, (socklen_t *)&tamanhoEndereco);

        if (socketCliente == -1)
        {
            perror("Erro ao aceitar conexão");
            continue;
        }

        // alocar memoria para args
        struct ThreadCliente *args = malloc(sizeof(struct ThreadCliente));
        if (!args)
        {
            perror("Erro ao alocar memória para args-thread");
            close(socketCliente);
            continue;
        }

        pthread_mutex_lock(&mutexClienteID);
        idCliente++;
        args->clienteId = idCliente;
        pthread_mutex_unlock(&mutexClienteID);

        args->socketCliente = socketCliente;
        args->server = server;

        // Criar thread para permitir clientes
        pthread_t thread;
        if (pthread_create(&thread, NULL, criaClienteThread, args) != 0)
        {
            perror("Failed to create thread");
            free(args);
            close(socketCliente);
            continue;
        }
        // Libertar recursos da thread

        pthread_detach(thread);
    }

    close(socketServidor);
}

// Fazer Parse Mensagem
struct FormatoMensagens parseMensagem(char *buffer)
{
    struct FormatoMensagens msg;
    msg.idCliente = strtok(buffer, "|");
    msg.tipoJogo = strtok(NULL, "|");
    msg.tipoResolucao = strtok(NULL, "|");
    msg.temJogo = strtok(NULL, "|");
    msg.idJogo = strtok(NULL, "|");
    msg.jogo = strtok(NULL, "|");
    msg.valoresCorretos = strtok(NULL, "|");
    msg.tempoInicio = strtok(NULL, "|");
    msg.tempoFinal = strtok(NULL, "|");
    msg.resolvido = strtok(NULL, "|");
    msg.numeroTentativas = strtok(NULL, "|");
    return msg;
}

// Validar msg
bool validarMensagemVazia(struct FormatoMensagens *msg)
{
    return !(CHECK_NULL(msg->idCliente) ||
             CHECK_NULL(msg->tipoJogo) ||
             CHECK_NULL(msg->tipoResolucao) ||
             CHECK_NULL(msg->temJogo) ||
             CHECK_NULL(msg->idJogo) ||
             CHECK_NULL(msg->jogo) ||
             CHECK_NULL(msg->valoresCorretos) ||
             CHECK_NULL(msg->tempoInicio) ||
             CHECK_NULL(msg->tempoFinal) ||
             CHECK_NULL(msg->resolvido) ||
             CHECK_NULL(msg->numeroTentativas));
}

// Função que dá handle a entrada de um cliente na fila e sua atribuição a uma sala
struct SalaSinglePlayer *handleSinglePlayerFila(int idCliente, struct ServidorConfig *serverConfig)
{
    // Tenta adicionar o cliente à fila de espera
    if (!enqueue(filaClientesSinglePlayer, idCliente))
    {

        printf("[Sistema] Cliente %d rejeitado \n",
               idCliente);
        return NULL;
    }

    // Incrementa o semáforo customers, sinalizando às salas
    // que existe um novo cliente esperando na fila
    // O sem_post acorda uma das threads barbeiro que esteja esperando em sem_wait
    sem_post(&filaClientesSinglePlayer->customers);

    // Espera ser atribuido a uma sala
    while (1)
    {
        // Percorre todas as salas disponíveis procurando por a sala que
        // aceitou o cliente
        for (int i = 0; i < NUM_JOGOS; i++)
        {
            // Bloqueia o mutex da sala para verificar seu estado
            pthread_mutex_lock(&serverConfig->sala[i].mutexSala);

            // Verifica duas condições:
            // 1. Se o ID do cliente atual da sala corresponde a este cliente
            // 2. Se a sala está ocupada (nClientes == 1)
            // Significa que o barbeiro selecionou esse cliente
            if (serverConfig->sala[i].clienteAtualID == idCliente &&
                serverConfig->sala[i].nClientes == 1)
            {
                // Libera o mutex antes de retornar
                pthread_mutex_unlock(&serverConfig->sala[i].mutexSala);

                return &serverConfig->sala[i];
            }

            // Libera o mutex se a sala não é a correta
            pthread_mutex_unlock(&serverConfig->sala[i].mutexSala);
        }

        // Pequena pausa para evitar consumo excessivo de CPU
        // enquanto espera por uma sala
        // 100ms
        usleep(100000);
    }
    return NULL;
}

void updateClientConfig(struct ClienteConfig *clienteConfig,
                        const struct FormatoMensagens *msg,
                        const char *jogoADar,
                        int nJogo)
{
    strcpy(clienteConfig->tipoJogo, msg->tipoJogo);
    strcpy(clienteConfig->tipoResolucao, msg->tipoResolucao);
    strcpy(clienteConfig->TemJogo, "COM_JOGO");
    strcpy(clienteConfig->jogoAtual.jogo, jogoADar);
    strcpy(clienteConfig->jogoAtual.valoresCorretos, jogoADar);
    strcpy(clienteConfig->jogoAtual.tempoInicio, getTempoHoraMinutoSegundo());
    strcpy(clienteConfig->jogoAtual.tempoFinal, msg->tempoFinal);
    clienteConfig->jogoAtual.resolvido = atoi(msg->resolvido);
    clienteConfig->jogoAtual.numeroTentativas = atoi(msg->numeroTentativas);
    clienteConfig->jogoAtual.idJogo = nJogo;
    clienteConfig->idCliente = atoi(msg->idCliente);
}

void bufferParaStructCliente(char *buffer, const struct ClienteConfig *clienteConfig)
{
    sprintf(buffer, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d",
            clienteConfig->idCliente,
            clienteConfig->tipoJogo,
            clienteConfig->tipoResolucao,
            clienteConfig->TemJogo,
            clienteConfig->jogoAtual.idJogo,
            clienteConfig->jogoAtual.jogo,
            clienteConfig->jogoAtual.valoresCorretos,
            clienteConfig->jogoAtual.tempoInicio,
            clienteConfig->jogoAtual.tempoFinal,
            clienteConfig->jogoAtual.resolvido,
            clienteConfig->jogoAtual.numeroTentativas);
}

char *handleResolucaoJogo(struct ClienteConfig *clienteConfig, struct SalaSinglePlayer *sala)
{
    char *logClienteEnviar = NULL;

    if (clienteConfig->jogoAtual.resolvido)
    {
        return 1;
    }

    if (strcmp(clienteConfig->tipoResolucao, "COMPLET") == 0 ||
        strcmp(clienteConfig->tipoResolucao, "PARCIAL") == 0)
    {
        logClienteEnviar = atualizaValoresCorretos(
            clienteConfig->jogoAtual.jogo,
            clienteConfig->jogoAtual.valoresCorretos,
            sala->jogo.solucao,
            &clienteConfig->jogoAtual.numeroTentativas);
    }

    return logClienteEnviar ? logClienteEnviar : "";
}

bool verSeJogoAcabouEAtualizar(struct ClienteConfig *cliente, struct SalaSinglePlayer *sala)
{
    pthread_mutex_lock(&sala->mutexSala);

    if (cliente->jogoAtual.resolvido)
    {
        pthread_mutex_unlock(&sala->mutexSala);
        return true;
    }

    if (verificaResolvido(cliente->jogoAtual.valoresCorretos, sala->jogo.solucao, true))
    {
        cliente->jogoAtual.resolvido = 1;
        strcpy(cliente->jogoAtual.tempoFinal, getTempoHoraMinutoSegundo());

        sala->jogadorAResolver = false;
        sala->nClientes = 0;

        time_t tempoInicioConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoInicio);
        time_t tempoFinalConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoFinal);
        time_t tempoDemorado = difftime(tempoFinalConvertido, tempoInicioConvertido);

        printf("Sala %d: Cliente %d resolveu o jogo %d em %ld segundos!\n",
               sala->idSala, cliente->idCliente, sala->jogo.idJogo, tempoDemorado);

        logQueEventoServidor(8, cliente->idCliente);

        pthread_mutex_unlock(&sala->mutexSala);
        return true;
    }

    pthread_mutex_unlock(&sala->mutexSala);
    return false;
}

void receberMensagemETratarServer(char *buffer, int socketCliente,
                                  struct ClienteConfig clienteConfig,
                                  struct ServidorConfig serverConfig)
{
    struct SalaSinglePlayer *salaAtual = NULL;
    char *jogoADar = "";
    int nJogo = -1;
    while (recv(socketCliente, buffer, BUF_SIZE, 0) > 0)
    {
        char bufferFinal[BUF_SIZE] = {0};
        sprintf(bufferFinal, "Mensagem recebida: %s\n", buffer);
        logEventoServidor(bufferFinal);

        struct FormatoMensagens msgData = parseMensagem(buffer);

        if (!validarMensagemVazia(&msgData))
        {
            printf("Erro: Mensagem recebida com formato inválido\n");
            break;
        }

        if (strcmp(msgData.temJogo, "SEM_JOGO") == 0)
        {

            if (strcmp(msgData.tipoJogo, "SIG") == 0)
            {
                salaAtual = handleSinglePlayerFila(atoi(msgData.idCliente), &serverConfig);
                if (!salaAtual)
                {
                    break;
                    // printf("Erro: Cliente %d não pode ser atendido no momento\n", atoi(msgData.idCliente));

                    // char queueMessage[BUF_SIZE];
                    // sprintf(queueMessage, "%d|SIG|WAIT|SEM_JOGO|0|0|0|0|0|0|0",
                    //         atoi(msgData.idCliente));
                    // writeSocket(socketCliente, queueMessage, BUF_SIZE);
                    // continue;
                }

                jogoADar = salaAtual->jogo.jogo;
                nJogo = salaAtual->jogo.idJogo;
            }

            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo);

            memset(buffer, 0, BUF_SIZE);
            bufferParaStructCliente(buffer, &clienteConfig);

            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0)
            {
                perror("Erro ao enviar mensagem para o cliente");
                break;
            }

            printf("Cliente-%d conectado à sala %d com jogo %d\n", clienteConfig.idCliente, salaAtual->idSala, nJogo);
            char bufferEnviarFinal[BUF_SIZE] = {0};
            sprintf(bufferEnviarFinal, "Mensagem enviada: %s\n", buffer);
            logQueEventoServidor(4, clienteConfig.idCliente);
            logEventoServidor(bufferEnviarFinal);
            memset(buffer, 0, BUF_SIZE);
        }

        if (strcmp(msgData.temJogo, "COM_JOGO") == 0)
        {
            clienteConfig.idCliente = atoi(msgData.idCliente);
            strcpy(clienteConfig.TemJogo, msgData.temJogo);
            strcpy(clienteConfig.tipoJogo, msgData.tipoJogo);
            strcpy(clienteConfig.tipoResolucao, msgData.tipoResolucao);
            clienteConfig.jogoAtual.idJogo = nJogo;
            strcpy(clienteConfig.jogoAtual.jogo, msgData.jogo);
            strcpy(clienteConfig.jogoAtual.valoresCorretos, msgData.valoresCorretos);
            strcpy(clienteConfig.jogoAtual.tempoInicio, msgData.tempoInicio);
            strcpy(clienteConfig.jogoAtual.tempoFinal, msgData.tempoFinal);
            clienteConfig.jogoAtual.resolvido = atoi(msgData.resolvido);
            clienteConfig.jogoAtual.numeroTentativas = atoi(msgData.numeroTentativas);

            logQueEventoServidor(5, clienteConfig.idCliente);

            char *logClienteEnviar = handleResolucaoJogo(&clienteConfig, salaAtual);
            bool gameCompleted = verSeJogoAcabouEAtualizar(&clienteConfig, salaAtual);

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

            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0)
            {
                perror("Erro ao enviar mensagem para o cliente");
                free(logClienteEnviar);
                break;
            }

            free(logClienteEnviar);

            char bufferFinal[BUF_SIZE] = {0};
            sprintf(bufferFinal, "Mensagem enviada: %s\n", buffer);
            logQueEventoServidor(6, clienteConfig.idCliente);
            logEventoServidor(bufferFinal);
            memset(buffer, 0, BUF_SIZE);
            if (gameCompleted)
            {
                printf("Jogo concluído na sala %d. Encerrando sessão do cliente %d.\n",
                       salaAtual->idSala, clienteConfig.idCliente);
                // 20% de entrar na sala outra vez
                // if (entrarNaSalaOutraVez() <= 20)
                // {
                //     printf("Jogador %d voltou a entrar na fila de jogos singleplayer\n", clienteConfig.idCliente);
                // }
                break;
            }
        }
    }
    if (salaAtual)
    {
        pthread_mutex_lock(&salaAtual->mutexSala);
        salaAtual->nClientes = 0;
        salaAtual->clienteAtualID = -1;
        pthread_mutex_unlock(&salaAtual->mutexSala);
        printf("Cliente %d desconectado da sala %d\n", clienteConfig.idCliente, salaAtual->idSala);
    }
    char temp[BUF_SIZE] = {0};
    sprintf(temp, "Cliente-%d desconectado", clienteConfig.idCliente);
    printf("%s\n", temp);
    logQueEventoServidor(7, clienteConfig.idCliente);
}

struct filaClientesSinglePlayer *criarFila()
{
    struct filaClientesSinglePlayer *fila = (struct filaClientesSinglePlayer *)malloc(sizeof(struct filaClientesSinglePlayer));
    if (!fila)
    {
        perror("Erro ao alocar memoria para fila");
        return NULL;
    }

    fila->clientesID = (int *)malloc(sizeof(int) * NUM_MAX_CLIENTES_FILA_SINGLEPLAYER);
    if (!fila->clientesID)
    {
        free(fila);
        perror("Erro ao alocar memoria para clientesID");
        return NULL;
    }

    fila->front = 0;
    fila->rear = -1;
    fila->tamanho = 0;
    fila->capacidade = NUM_MAX_CLIENTES_FILA_SINGLEPLAYER;

    if (pthread_mutex_init(&fila->mutex, NULL) != 0)
    {
        free(fila->clientesID);
        free(fila);
        perror("Erro ao inicializar mutex");
        return NULL;
    }

    // Semaforo para os clientes notificarem as salas
    if (sem_init(&fila->customers, 0, 0) != 0)
    {
        pthread_mutex_destroy(&fila->mutex);
        free(fila->clientesID);
        free(fila);
        perror("Erro ao inicializar semaforo");
        return NULL;
    }

    return fila;
}

void delete_queue(struct filaClientesSinglePlayer *fila)
{
    if (fila)
    {
        sem_destroy(&fila->customers);
        pthread_mutex_destroy(&fila->mutex);
        if (fila->clientesID)
        {
            free(fila->clientesID);
        }
        free(fila);
    }
}

bool boolEstaFilaCheia(struct filaClientesSinglePlayer *fila)
{
    return fila->tamanho == fila->capacidade;
}

bool estaFilaCheiaTrSafe(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);
    bool full = boolEstaFilaCheia(fila);
    pthread_mutex_unlock(&fila->mutex);
    return full;
}

bool boolEstaFilaVazia(struct filaClientesSinglePlayer *fila)
{
    return fila->tamanho == 0;
}

bool estaFilaVaziaTrSafe(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);
    bool empty = boolEstaFilaVazia(fila);
    pthread_mutex_unlock(&fila->mutex);
    return empty;
}

bool enqueue(struct filaClientesSinglePlayer *fila, int clientID)
{
    pthread_mutex_lock(&fila->mutex);

    if (fila->tamanho >= fila->capacidade)
    {
        printf("[Fila] Rejeitado cliente %d - fila cheia (tamanho: %d)\n",
               clientID, fila->tamanho);
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    fila->rear = (fila->rear + 1) % fila->capacidade;
    fila->clientesID[fila->rear] = clientID;
    fila->tamanho++;

    printf("[Fila] Cliente %d entrou na fila (posição: %d, tamanho: %d)\n",
           clientID, fila->rear, fila->tamanho);

    pthread_mutex_unlock(&fila->mutex);
    return true;
}

int dequeue(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);

    if (fila->tamanho == 0)
    {
        pthread_mutex_unlock(&fila->mutex);
        return -1; // Queue is empty
    }

    // Save the client ID of the first element
    int clientID = fila->clientesID[0];

    // Shift all the elements to the left
    for (int i = 1; i < fila->tamanho; i++)
    {
        fila->clientesID[i - 1] = fila->clientesID[i];
    }

    // Decrease the size of the queue
    fila->tamanho--;

    // Update rear pointer
    fila->rear = fila->tamanho - 1;

    printf("[Fila] Cliente %d removido da fila (tamanho restante: %d)\n",
           clientID, fila->tamanho);

    pthread_mutex_unlock(&fila->mutex);
    return clientID;
}
void print_fila(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);

    if (boolEstaFilaVazia(fila))
    {
        printf("Fila vazia\n");
        pthread_mutex_unlock(&fila->mutex);
        return;
    }

    int count = fila->tamanho;
    int indice = fila->front;

    printf("Tamanho fila: %d\n", fila->tamanho);
    printf("Conteudo fila:\n");

    while (count > 0)
    {
        printf("%d ", fila->clientesID[indice]);
        indice = (indice + 1) % fila->capacidade;
        count--;
    }
    printf("\n");

    pthread_mutex_unlock(&fila->mutex);
}

int getFilaTamanho(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);
    int size = fila->tamanho;
    pthread_mutex_unlock(&fila->mutex);
    return size;
}

void *Sala(void *arg)
{
    struct SalaSinglePlayer *sala = (struct SalaSinglePlayer *)arg;
    // struct filaClientesSinglePlayer *fila = ;

    printf("[Sala-%d] Iniciado\n", sala->idSala);

    while (1)
    {
        // Sala pronta para proximo cliente
        sem_wait(&filaClientesSinglePlayer->customers);

        pthread_mutex_lock(&sala->mutexSala);
        if (sala->nClientes > 0)
        {
            pthread_mutex_unlock(&sala->mutexSala);
            sem_post(&filaClientesSinglePlayer->customers);
            continue;
        }

        // Proximo cliente na fila
        int clienteID = dequeue(filaClientesSinglePlayer);
        if (clienteID == -1)
        {
            pthread_mutex_unlock(&sala->mutexSala);
            sem_post(&filaClientesSinglePlayer->customers);
            continue;
        }

        // Sala tem agora cliente
        sala->nClientes = 1;
        sala->jogadorAResolver = true;
        sala->clienteAtualID = clienteID;

        // Preparar jogo
        // unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();
        // int jogoIndex = rand_r(&seed) % NUM_JOGOS;
        // sala->jogo = jogosEsolucoes[jogoIndex];

        printf("[Sala-%d] Atendendo cliente %d com jogo %d\n",
               sala->idSala, clienteID, sala->jogo.idJogo);

        pthread_mutex_unlock(&sala->mutexSala);

        while (1)
        {
            pthread_mutex_lock(&sala->mutexSala);
            if (!sala->jogadorAResolver)
            {
                // reset depois cliente acabar
                sala->nClientes = 0;
                sala->clienteAtualID = -1;
                printf("[Sala-%d] Cliente %d finalizou\n",
                       sala->idSala, clienteID);
                pthread_mutex_unlock(&sala->mutexSala);

                break;
            }
            pthread_mutex_unlock(&sala->mutexSala);
            // 100ms para cpu nao ficar sobrecarregado
            usleep(100000);
        }
    }

    return NULL;
}
struct SalaSinglePlayer *criarSalaSinglePlayer(int idSala)
{
    struct SalaSinglePlayer *sala = malloc(sizeof(struct SalaSinglePlayer));
    if (!sala)
    {
        perror("Erro ao alocar memória para sala");
        return NULL;
    }

    sala->idSala = idSala;
    sala->clientesMax = 1;
    sala->clienteMin = 1;
    sala->nClientes = 0;
    sala->jogadorAResolver = false;

    if (pthread_mutex_init(&sala->mutexSala, NULL) != 0)
    {
        free(sala);
        return NULL;
    }

    return sala;
}

void *iniciarSalaSinglePlayer(void *arg)
{
    struct SalaSinglePlayer *sala = (struct SalaSinglePlayer *)arg;
    Sala(sala);
    return NULL;
}
void iniciarSalasJogoSinglePlayer(struct ServidorConfig *serverConfig)
{
    printf("[Sistema] Iniciando %d salas\n", NUM_JOGOS);

    if (!serverConfig || !serverConfig->sala)
    {
        perror("Server config or rooms not initialized");
        exit(1);
    }

    for (int i = 0; i < NUM_JOGOS; i++)
    {
        serverConfig->sala[i].idSala = i;
        serverConfig->sala[i].clientesMax = 1;
        serverConfig->sala[i].clienteMin = 1;
        serverConfig->sala[i].nClientes = 0;
        serverConfig->sala[i].jogadorAResolver = false;
        serverConfig->sala[i].clienteAtualID = -1;

        // Cada sala tem um jogo
        serverConfig->sala[i].jogo = jogosEsolucoes[i];

        if (pthread_mutex_init(&serverConfig->sala[i].mutexSala, NULL) != 0)
        {
            perror("Failed to initialize room mutex");
            exit(1);
        }

        // Umasala é uma tread
        pthread_t threadSala;
        void *roomPtr = &serverConfig->sala[i];
        if (pthread_create(&threadSala, NULL, iniciarSalaSinglePlayer, roomPtr) != 0)
        {
            perror("Failed to create barber thread");
            exit(1);
        }
        pthread_detach(threadSala);
    }
}
int entrarNaSalaOutraVez()
{
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)pthread_self();
    int probalidade = rand_r(&seed) % 100;
    return probalidade;
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
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho);
    serverConfig = construtorServer(AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 1, serverConfig.ficheiroJogosESolucoesCaminho);
    logQueEventoServidor(1, 0);
    filaClientesSinglePlayer = malloc(sizeof(struct filaClientesSinglePlayer));
    filaClientesSinglePlayer = criarFila();
    iniciarServidorSocket(&serverConfig);
    return 0;
}