#include "../headers/servidor.h"

struct filaClientesSinglePlayer *filaClientesSinglePlayer;

// globais
static int idCliente = 0;

// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNTentativas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIDSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aceitarCliente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verificaEstadoSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAcabouJogo = PTHREAD_MUTEX_INITIALIZER;
// semaforos
sem_t semaforoSingleJogador;
//carregar configs do servidor

void carregarConfigServidor(char *nomeFicheiro, struct ServidorConfig *serverConfig)
{
    FILE *config = abrirFicheiroRead(nomeFicheiro);

    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    //incrementa contadorConfigs e vai lendo cada linha do ficheiro e assoacia a respetivo campo da struct
    while (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        switch (contadorConfigs)
        {
        case 0:
            strcpy(serverConfig->ficheiroJogosESolucoesCaminho, strtok(buffer, "\n"));
            break;
        case 1:
            serverConfig->porta = (unsigned int) strtoul(strtok(buffer, "\n"), NULL, 10);
            break;
        case 2:
            serverConfig->NUM_MAX_CLIENTES_FILA_SINGLEPLAYER = atoi(strtok(buffer, "\n"));
            break;
        default:
            break;
        }
        contadorConfigs++;
    }
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
void logQueEventoServidor(int numero, int clienteID,int salaID)
{
    char *mensagem = malloc(BUF_SIZE * sizeof(char));
    if (mensagem == NULL)
    {
        perror("Erro ao alocar memoria para mensagem");
        exit(1);
    }

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
    case 9:
        sprintf(mensagem, "Cliente-%d entrou na fila", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 10:
        sprintf(mensagem, "Cliente-%d removido da fila", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 11:
        sprintf(mensagem, "Cliente-%d rejeitado a entrar na fila", clienteID);
        logEventoServidor(mensagem);
        free(mensagem);
        break;
    case 12:
        sprintf(mensagem, "Cliente-%d entrou na sala-%d", clienteID,salaID);
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
    for (int i = 0; i < (int) strlen(tentativaAtual); i++)
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
bool verificaResolvido(char valoresCorretos[], char solucao[])
{
    for (int i = 0; i < (int) strlen(valoresCorretos); i++)
    {
        if (valoresCorretos[i] != solucao[i])
        {
            return false;
        }
    }
    return true;
}

int lerNumeroJogos(char *nomeFicheiro)
{
    FILE *config = abrirFicheiroRead(nomeFicheiro);
    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    int numeroJogos = 0;
    while (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        if (contadorConfigs % 3 == 2) {
            numeroJogos++;
        }
        contadorConfigs++;
    }
    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(1);
    }
    return numeroJogos;
}

void carregarFicheiroJogosSolucoes(char *nomeFicheiro, struct Jogo jogosEsolucoes[]) {
    FILE *config = abrirFicheiroRead(nomeFicheiro);
    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    int linhaAtual = 0;
    
    // le todos os jogos
    while (fgets(buffer, BUF_SIZE, config) != NULL) {
        switch(linhaAtual) {
            case 0:  // ID do jogo
                jogosEsolucoes[contadorConfigs].idJogo = atoi(strtok(buffer, "\n"));
                linhaAtual++;
                break;
                
            case 1:  // Estado inicial do jogo
                strcpy(jogosEsolucoes[contadorConfigs].jogo, strtok(buffer, "\n"));
                linhaAtual++;
                break;
                
            case 2:  // Solução do jogo
                strcpy(jogosEsolucoes[contadorConfigs].solucao, strtok(buffer, "\n"));
                linhaAtual = 0;  // Reset para próximo jogo
                contadorConfigs++;  // Incrementa apenas quando um jogo completo é lido
                break;
            default:
                perror("Erro ao ler ficheiro de jogos e soluções");
                fecharFicheiro(config);
                exit(1);
        }
    }
    
    fecharFicheiro(config);
    
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
}

void construtorServer(struct ServidorConfig *servidor,
                     int dominio, 
                     int servico, 
                     int protocolo, 
                     __u_long interface, 
                     int porta, 
                     int backlog, 
                     char *ficheiroJogosESolucoesCaminho) {
    if (!servidor || !ficheiroJogosESolucoesCaminho) {
        perror("Erro ao criar servidor");
        exit(1);
    }

    servidor->dominio = dominio;
    servidor->servico = servico;
    servidor->protocolo = protocolo;
    servidor->interface = interface;
    servidor->porta = porta;
    servidor->backlog = backlog;

    strncpy(servidor->ficheiroJogosESolucoesCaminho, 
            ficheiroJogosESolucoesCaminho, 
            PATH_SIZE - 1);
    servidor->ficheiroJogosESolucoesCaminho[PATH_SIZE - 1] = '\0';

    //alocar memoria para as salas
    servidor->sala = malloc(servidor->numeroJogos * sizeof(struct SalaSinglePlayer));
    if (!servidor->sala) {
        perror("Erro ao alocar memória para salas");
        exit(1);
    }

    //colocar a 0 as posicoes de mem que temos
    memset(servidor->sala, 0, servidor->numeroJogos * sizeof(struct SalaSinglePlayer));

    //-1 = nenhum
    for (int i = 0; i < servidor->numeroJogos; i++) {
        servidor->sala[i].idSala = -1;
        servidor->sala[i].nClientes = 0;
        servidor->sala[i].jogadorAResolver = false;
        servidor->sala[i].clienteAtualID = -1;
    }
}

//Sempre que um cliente faz a ligacao ao servidor, é criada uma thread para ele
// ficando o cliente na funcao recebeMensagemETratarServer
// ate acabar o jogo, depois de acabar é fechado o socket e a thread
void *criaClienteThread(void *arg)
{
    struct ThreadCliente *args = (struct ThreadCliente *)arg;
    int socketCliente = args->socketCliente;

    pthread_mutex_lock(&mutexClienteID);
    idCliente++;
    args->clienteId = idCliente;
   

    int clientID = args->clienteId;

    struct ClienteConfig clienteConfig = {0};

    char temp[BUF_SIZE] = {0};
    
    
    int bytesRecebidos;
    while((bytesRecebidos = readSocket(socketCliente, temp, BUF_SIZE)) > 0){
        if(strstr(temp, "MANDA_ID") != NULL){
            //envia o id do cliente para o cliente
            // ID|
            sprintf(temp, "%d|", clientID);
            if(writeSocket(socketCliente, temp, BUF_SIZE) < 0){
                perror("Erro ao enviar ID");
                exit(1);
            }
            //passo 0 como sala nao importa para esta msg
            logQueEventoServidor(3, clientID,0);
            break;
        }
    }
    if(bytesRecebidos == 0){
        printf("Erro parte dos IDs\n");
        exit(1);
    }
    
    pthread_mutex_unlock(&mutexClienteID);
    receberMensagemETratarServer(temp, socketCliente, clienteConfig, *args->server);
    shutdown(socket, SHUT_WR);
    close(socketCliente);
    free(args);
    return NULL;
}

/*
    * Inicia o servidor socket
    * 
    * @param server - Configurações do servidor
    * @param jogosEsolucoes - Array de jogos e soluções
    * 
    * Trata de iniciar o servidor socket
    * iniciar fila para sala singleplayer
    * aceitar conexoes, criando tarefas
    * 
*/

void iniciarServidorSocket(struct ServidorConfig *server,struct Jogo jogosEsolucoes[])
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

    sem_init(&semaforoSingleJogador,1,server->numeroJogos);
    //inicializa salas dos dois tipos de jogo singleplayer e multiplayer
    //cada sala é uma tarefa
    iniciarSalasJogoSinglePlayer(server,jogosEsolucoes);
    iniciarSalasJogoMultiplayer(server,jogosEsolucoes);
    while (1)
    {
        struct sockaddr_in enderecoCliente;
        int tamanhoEndereco = sizeof(enderecoCliente);
        pthread_mutex_lock(&aceitarCliente);
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
            shutdown(socket, SHUT_WR);
            close(socketCliente);
            continue;
        }

        
        args->socketCliente = socketCliente;
        args->server = server;

        // Criar thread para permitir clientes
        pthread_t thread;
        pthread_mutex_unlock(&aceitarCliente);
        
        if (pthread_create(&thread, NULL, criaClienteThread, args) != 0)
        {
            perror("Failed to create thread");
            free(args);
            shutdown(socket, SHUT_WR);
            close(socketCliente);
            continue;
        }
        // Liberta recursos da thread
        pthread_detach(thread);
    }
    shutdown(socket, SHUT_WR);
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
    msg.idSala = strtok(NULL, "|");
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
struct SalaSinglePlayer* handleSinglePlayerFila(int idCliente, struct ServidorConfig* serverConfig) {
    if (!enqueue(filaClientesSinglePlayer, idCliente)) {
        return NULL;
    }
    
    sem_post(&filaClientesSinglePlayer->customers);
    sem_wait(&semaforoSingleJogador);
    // printf("Cliente %d a verificar se foi chamado\n", idCliente);
    
    struct SalaSinglePlayer* salaEncontrada = NULL;
    
    while (!salaEncontrada) {
        pthread_mutex_lock(&verificaEstadoSala);
        
        for (int i = 0; i < serverConfig->numeroJogos; i++) {
            if (serverConfig->sala[i].clienteAtualID == idCliente) {
                salaEncontrada = &serverConfig->sala[i];
                // printf("Cliente %d foi chamado para a sala %d\n", idCliente, i);
                break;
            }
        }
        
        pthread_mutex_unlock(&verificaEstadoSala);
        
        if (!salaEncontrada) {
            usleep(10000);
        }
    }
    
    sem_post(&semaforoSingleJogador);
    return salaEncontrada;
}


void updateClientConfig(struct ClienteConfig *clienteConfig,
                        const struct FormatoMensagens *msg,
                        const char *jogoADar,
                        int nJogo,
                        int salaID)
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
    clienteConfig->idSala = salaID;
    clienteConfig->idCliente = atoi(msg->idCliente);
}

void bufferParaStructCliente(char *buffer, const struct ClienteConfig *clienteConfig)
{
    sprintf(buffer, "%u|%s|%s|%s|%d|%d|%s|%s|%s|%s|%d|%d",
            clienteConfig->idCliente,
            clienteConfig->tipoJogo,
            clienteConfig->tipoResolucao,
            clienteConfig->TemJogo,
            clienteConfig->jogoAtual.idJogo,
            clienteConfig->idSala,
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
        return "1";
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
    pthread_mutex_lock(&mutexAcabouJogo);

    if (cliente->jogoAtual.resolvido)
    {
        pthread_mutex_unlock(&mutexAcabouJogo);
        return true;
    }

    if (verificaResolvido(cliente->jogoAtual.valoresCorretos, sala->jogo.solucao))
    {
        cliente->jogoAtual.resolvido = 1;
        strcpy(cliente->jogoAtual.tempoFinal, getTempoHoraMinutoSegundo());

        sala->jogadorAResolver = false;
        sala->nClientes = 0;
        sala->clienteAtualID = -1;
        time_t tempoInicioConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoInicio);
        time_t tempoFinalConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoFinal);
        time_t tempoDemorado = difftime(tempoFinalConvertido, tempoInicioConvertido);

        printf("[Sala-%d] Cliente %d resolveu o jogo %d em %ld segundos!\n",
               sala->idSala, cliente->idCliente, sala->jogo.idJogo, tempoDemorado);
        //passo 0 como nao importa para esta msg
        logQueEventoServidor(8, cliente->idCliente,0);

        pthread_mutex_unlock(&mutexAcabouJogo);
        return true;
    }

    pthread_mutex_unlock(&mutexAcabouJogo);
    return false;
}
void atualizarClientConfig(struct ClienteConfig *clienteConfig, const struct FormatoMensagens *msgData) {
    clienteConfig->idCliente = atoi(msgData->idCliente);
    strcpy(clienteConfig->TemJogo, msgData->temJogo);
    strcpy(clienteConfig->tipoJogo, msgData->tipoJogo);
    strcpy(clienteConfig->tipoResolucao, msgData->tipoResolucao);
    clienteConfig->jogoAtual.idJogo = atoi(msgData->idJogo);
    clienteConfig->idSala = atoi(msgData->idSala);
    strcpy(clienteConfig->jogoAtual.jogo, msgData->jogo);
    strcpy(clienteConfig->jogoAtual.valoresCorretos, msgData->valoresCorretos);
    strcpy(clienteConfig->jogoAtual.tempoInicio, msgData->tempoInicio);
    strcpy(clienteConfig->jogoAtual.tempoFinal, msgData->tempoFinal);
    clienteConfig->jogoAtual.resolvido = atoi(msgData->resolvido);
    clienteConfig->jogoAtual.numeroTentativas = atoi(msgData->numeroTentativas);
}

// Helper function to prepare game response
void prepararRespostaJogo(char *buffer, const struct ClienteConfig *clienteConfig, const char *logClienteEnviar) {
    snprintf(buffer, BUF_SIZE, "%u|%s|%s|%s|%d|%d|%s|%s|%s|%s|%d|%d|%s",
            clienteConfig->idCliente,
            clienteConfig->tipoJogo,
            clienteConfig->tipoResolucao,
            clienteConfig->TemJogo,
            clienteConfig->jogoAtual.idJogo,
            clienteConfig->idSala,
            clienteConfig->jogoAtual.jogo,
            clienteConfig->jogoAtual.valoresCorretos,
            clienteConfig->jogoAtual.tempoInicio,
            clienteConfig->jogoAtual.tempoFinal,
            clienteConfig->jogoAtual.resolvido,
            clienteConfig->jogoAtual.numeroTentativas,
            logClienteEnviar ? logClienteEnviar : "");
}
void receberMensagemETratarServer(char *buffer, int socketCliente,
                                 struct ClienteConfig clienteConfig,
                                 struct ServidorConfig serverConfig)
{
    struct SalaSinglePlayer *salaAtual = NULL;
    char *jogoADar = "";
    int nJogo = -1;
    int bytesRecebidos;
    int SalaID = -1;
    bool clienteDesconectado = false;

    while (!clienteDesconectado && (bytesRecebidos = readSocket(socketCliente, buffer, BUF_SIZE)) > 0) {
        // Log received message
        char bufferFinal[BUF_SIZE] = {0};
        snprintf(bufferFinal, BUF_SIZE, "Mensagem recebida: %s", buffer);
        logEventoServidor(bufferFinal);

        struct FormatoMensagens msgData = parseMensagem(buffer);
        if (!validarMensagemVazia(&msgData)) {
            printf("Erro: Mensagem recebida com formato inválido\n");
            break;
        }

        // Handle initial game request
        if (strcmp(msgData.temJogo, "SEM_JOGO") == 0) {
            if (strcmp(msgData.tipoJogo, "SIG") == 0) {
                salaAtual = handleSinglePlayerFila(atoi(msgData.idCliente), &serverConfig);
                if (!salaAtual) {
                    const char* filaCheia = "FILA CHEIA SINGLEPLAYER";
                    send(socketCliente, filaCheia, strlen(filaCheia), 0);
                    clienteConfig.idCliente = atoi(msgData.idCliente);
                    clienteDesconectado = true;
                    break;
                }

                jogoADar = salaAtual->jogo.jogo;
                nJogo = salaAtual->jogo.idJogo;
                SalaID = salaAtual->idSala;
            }

            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo,SalaID);

            bufferParaStructCliente(buffer, &clienteConfig);

            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                perror("Erro ao enviar mensagem para o cliente");
                break;
            }

            logQueEventoServidor(4, clienteConfig.idCliente, salaAtual ? salaAtual->idSala : 0);
            
            char bufferEnviarFinal[BUF_SIZE] = {0};
            snprintf(bufferEnviarFinal, BUF_SIZE, "Mensagem enviada: %s", buffer);
            logEventoServidor(bufferEnviarFinal);
            
            memset(buffer, 0, BUF_SIZE);
            continue;
        }

        // Handle game moves
        if (strcmp(msgData.temJogo, "COM_JOGO") == 0) {
            if (!salaAtual) {
                printf("Erro: Cliente sem sala atribuída\n");
                break;
            }

            // Update client config with received data
            atualizarClientConfig(&clienteConfig, &msgData);
            logQueEventoServidor(5, clienteConfig.idCliente, salaAtual->idSala);

            char *logClienteEnviar = handleResolucaoJogo(&clienteConfig, salaAtual);
            bool gameCompleted = verSeJogoAcabouEAtualizar(&clienteConfig, salaAtual);

            // Prepare and send response
            memset(buffer, 0, BUF_SIZE);
            prepararRespostaJogo(buffer, &clienteConfig, logClienteEnviar);

            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                perror("Erro ao enviar mensagem para o cliente");
                break;
            }

            logQueEventoServidor(6, clienteConfig.idCliente, salaAtual->idSala);
            
            if (gameCompleted) {
                break;
            }
        }
    }
    printf(COLOR_RED "Cliente %d saiu\n" COLOR_RESET, clienteConfig.idCliente);
    // // Cleanup when client disconnects
    // if (salaAtual) {
    //     pthread_mutex_lock(&salaAtual->mutexSala);
    //     salaAtual->nClientes = 0;
    //     salaAtual->clienteAtualID = -1;
    //     salaAtual->jogadorAResolver = false;
    //     pthread_cond_broadcast(&salaAtual->condSala);
    //     pthread_mutex_unlock(&salaAtual->mutexSala);
        
    //     printf("Cliente %d saiu da sala %d\n", clienteConfig.idCliente, salaAtual->idSala);
    // }

    logQueEventoServidor(7, clienteConfig.idCliente, salaAtual ? salaAtual->idSala : 0);
}

struct filaClientesSinglePlayer *criarFila(struct ServidorConfig *serverConfig)
{
    struct filaClientesSinglePlayer *fila = (struct filaClientesSinglePlayer *)malloc(sizeof(struct filaClientesSinglePlayer));
    if (!fila)
    {
        perror("Erro ao alocar memoria para fila");
        return NULL;
    }
    fila->clientesID = (int *)malloc(sizeof(int) * serverConfig->NUM_MAX_CLIENTES_FILA_SINGLEPLAYER);
    if (!fila->clientesID)
    {
        free(fila);
        perror("Erro ao alocar memoria para clientesID");
        return NULL;
    }

    fila->front = 0;
    fila->rear = -1;
    fila->tamanho = 0;
    fila->capacidade = serverConfig->NUM_MAX_CLIENTES_FILA_SINGLEPLAYER;

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
               //passo 0 como nao importa para esta msg
        logQueEventoServidor(11, clientID,0);
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }

    fila->rear = (fila->rear + 1) % fila->capacidade;
    fila->clientesID[fila->rear] = clientID;
    fila->tamanho++;

    printf(COLOR_GREEN "[Fila] Cliente %d entrou na fila (posição: %d, tamanho: %d)\n"COLOR_RESET,
           clientID, fila->rear, fila->tamanho);
           //passo 0 como nao importa para esta msg
    logQueEventoServidor(9, clientID,0);
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

    printf(COLOR_RED "[Fila] Cliente %d removido da fila (tamanho restante: %d)\n"COLOR_RESET,
           clientID, fila->tamanho);
           //passo 0 como nao importa para esta msg
    logQueEventoServidor(10, clientID,0);
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

void removerFilaPorID(struct filaClientesSinglePlayer *fila, int clientID) {
    pthread_mutex_lock(&fila->mutex);
    
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->clientesID[i] == clientID) {
            // deslocar todos os elementos para a esquerda
            for (int j = i; j < fila->tamanho - 1; j++) {
                fila->clientesID[j] = fila->clientesID[j + 1];
            }
            fila->tamanho--;
            fila->rear = (fila->tamanho > 0) ? fila->tamanho - 1 : -1;
            
            pthread_mutex_unlock(&fila->mutex);
            printf("[Fila] Cliente %d removido da fila após desconexão\n", clientID);
            return;
        }
    }
    
    pthread_mutex_unlock(&fila->mutex);
    
}

void* SalaSingleplayer(void* arg) {
    struct SalaSinglePlayer* sala = (struct SalaSinglePlayer*)arg;
    printf("[Sala-%d] Iniciada-Singleplayer\n", sala->idSala);
    
    while (1) {
        sem_wait(&filaClientesSinglePlayer->customers);
        pthread_mutex_lock(&sala->mutexSala);
        
        if (sala->nClientes > 0) {
            pthread_mutex_unlock(&sala->mutexSala);
            sem_post(&filaClientesSinglePlayer->customers);
            continue;
        }
        
        int clienteID = dequeue(filaClientesSinglePlayer);
        if (clienteID == -1) {
            pthread_mutex_unlock(&sala->mutexSala);
            sem_post(&filaClientesSinglePlayer->customers);
            continue;
        }
        
        sala->nClientes = 1;
        sala->jogadorAResolver = true;
        sala->clienteAtualID = clienteID;
                
        printf(COLOR_YELLOW "[Sala-%d] Atendendo cliente %d com jogo %d\n" COLOR_RESET,
               sala->idSala, clienteID, sala->jogo.idJogo);
               
        bool clienteFinalizou = false;
        while (!clienteFinalizou) {
            
            if (!sala->jogadorAResolver) {
                sala->nClientes = 0;
                sala->clienteAtualID = -1;
                clienteFinalizou = true;
                printf("[Sala-%d] Cliente %d finalizou\n", sala->idSala, clienteID);
                printf("[Sala-%d] Cliente %d saiu da sala\n", sala->idSala, clienteID);
                pthread_mutex_unlock(&sala->mutexSala);
                break;
            }
                        
            if (!clienteFinalizou) {
                usleep(100000);
            }
        }
    }
    return NULL;
}
void *SalaMultiplayer(void *arg)
{
    struct SalaSinglePlayer *sala = (struct SalaSinglePlayer *)arg;
    // struct filaClientesSinglePlayer *fila = ;

    printf("[Sala-%d] Iniciada-Multiplayer\n", sala->idSala);
    return NULL;
}
void *iniciarSalaSinglePlayer(void *arg)
{
    struct SalaSinglePlayer *sala = (struct SalaSinglePlayer *)arg;
    SalaSingleplayer(sala);
    return NULL;
}
void *iniciarSalaMultiplayer(void *arg)
{
    struct SalaMultiplayer *sala = (struct SalaMultiplayer *)arg;
    SalaMultiplayer(sala);
    return NULL;
}
void iniciarSalasJogoSinglePlayer(struct ServidorConfig *serverConfig, struct Jogo jogosEsolucoes[])
{
    printf("[Sistema] Iniciando %d salas\n", 2 * serverConfig->numeroJogos);

    if (!serverConfig || !serverConfig->sala)
    {
        perror("Server config ou salas nao inicializadas");
        exit(1);
    }

    for (int i = 0; i < serverConfig->numeroJogos; i++)
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

void iniciarSalasJogoMultiplayer(struct ServidorConfig *serverConfig, struct Jogo jogosEsolucoes[])
{
    if (!serverConfig || !serverConfig->sala)
    {
        perror("Server config ou salas nao inicializadas");
        exit(1);
    }

    int numeroTotalSalas = 3 + serverConfig->numeroJogos;
    for (int i = 3; i < numeroTotalSalas; i++)
    {
        serverConfig->salaMultiplayer[i].idSala = i;
        serverConfig->salaMultiplayer[i].clientesMax = 5;
        serverConfig->salaMultiplayer[i].clienteMin = 5;
        serverConfig->salaMultiplayer[i].nClientes = 0;
        pthread_cond_init(&serverConfig->salaMultiplayer[i].jogoADecorrer, NULL);
        serverConfig->salaMultiplayer[i].jogoIniciado = false;
        serverConfig->salaMultiplayer[i].clienteAtualID = NULL;

        // Cada sala tem um jogo
        serverConfig->salaMultiplayer[i].jogo = jogosEsolucoes[i];

        if (pthread_mutex_init(&serverConfig->salaMultiplayer[i].mutexSala, NULL) != 0)
        {
            perror("Failed to initialize room mutex");
            exit(1);
        }
        if (pthread_cond_init(&serverConfig->salaMultiplayer[i].jogoADecorrer, NULL) != 0)
        {
            perror("Failed to initialize room mutex");
            exit(1);
        }

        // Umasala é uma tread
        pthread_t threadSala;
        void *roomPtr = &serverConfig->salaMultiplayer[i];
        if (pthread_create(&threadSala, NULL, iniciarSalaMultiplayer, roomPtr) != 0)
        {
            perror("Failed to create barber thread");
            exit(1);
        }
        pthread_detach(threadSala);
    }
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
    int numeroJogos = lerNumeroJogos(serverConfig.ficheiroJogosESolucoesCaminho);
    struct Jogo jogosEsolucoes[numeroJogos];
    serverConfig.numeroJogos = numeroJogos;
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho,jogosEsolucoes);
    construtorServer(&serverConfig,AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 1, serverConfig.ficheiroJogosESolucoesCaminho);
    //passo 0 como nao importa para esta msg
    logQueEventoServidor(1, 0,0);
    filaClientesSinglePlayer = malloc(sizeof(struct filaClientesSinglePlayer));
    filaClientesSinglePlayer = criarFila(&serverConfig);
    iniciarServidorSocket(&serverConfig,jogosEsolucoes);
    return 0;
}