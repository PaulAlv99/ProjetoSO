#include "../headers/servidor.h"
#define MAX_CLIENTS 5000  // Adjust based on your needs

struct filaClientesSinglePlayer *filaClientesSinglePlayer;

// globais
static int idCliente = 0;

// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIDSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aceitarCliente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verificaEstadoSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAcabouJogo = PTHREAD_MUTEX_INITIALIZER;
// semaforos
sem_t mutexSalaMultiplayer;
sem_t acessoLugares;
sem_t bloquearAcessoSala;
sem_t entradaPlayerMulPrimeiro;

pthread_barrier_t barreiraComecaTodos;
bool isSocketAlive(int socket) {
    int error = 0;
    socklen_t len = sizeof(error);
    int retval = getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len);
    
    if (retval != 0) {
        return false;
    }
    
    if (error != 0) {
        return false;
    }
    
    char buffer[1];
    int peek_result = recv(socket, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
    if (peek_result == 0) { 
        return false;
    } else if (peek_result < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
           
            return false;
        }
    }
    
    return true;
}

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

    *nTentativas = *nTentativas + 1;
    

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
        servidor->sala[i].clienteAtual.idCliente = -1;
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

    //inicializa salas dos dois tipos de jogo singleplayer e multiplayer
    //cada sala é uma tarefa
    //TODO mudar para apenas uma sala multiplayer
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
struct SalaSinglePlayer* handleSinglePlayerFila(struct ClienteConfig *cliente, struct ServidorConfig* serverConfig) {
    struct SalaSinglePlayer* salaEncontrada = NULL;
    //acesso aos lugares disponiveis
    sem_wait(&acessoLugares);
    if (!enqueue(filaClientesSinglePlayer, *cliente)) {
        sem_destroy(cliente->sinalizarVerificaSala);
        free(cliente->sinalizarVerificaSala);
        sem_post(&acessoLugares);
        return NULL;
    }
    sem_post(&acessoLugares);
    sem_post(&filaClientesSinglePlayer->customers);
    
    sem_wait(cliente->sinalizarVerificaSala);
    
    for (int i = 0; i < serverConfig->numeroJogos; i++) {
        if (serverConfig->sala[i].clienteAtual.idCliente == cliente->idCliente) {
            salaEncontrada = &serverConfig->sala[i];
            break;
        }
    }
    
    // Clean up semaphore after use
    sem_destroy(cliente->sinalizarVerificaSala);
    free(cliente->sinalizarVerificaSala);
    cliente->sinalizarVerificaSala = NULL;
    
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

    if (verificaResolvido(cliente->jogoAtual.valoresCorretos, sala->jogo.solucao))
    {
        cliente->jogoAtual.resolvido = 1;
        strcpy(cliente->jogoAtual.tempoFinal, getTempoHoraMinutoSegundo());

        sala->jogadorAResolver = false;
        sala->nClientes = 0;
        sala->clienteAtual.idCliente = -1;
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
    struct SalaSinglePlayer *salaAtualSIG = NULL;
    struct SalaMultiplayer* salaAtualMUL= NULL;
    char *jogoADar = "";
    int nJogo = -1;
    int bytesRecebidos;
    int SalaID = -1;
    bool clienteDesconectado = false;

    sem_t *clientSem = malloc(sizeof(sem_t));
    if (!clientSem) {
        perror("Failed to allocate semaphore");
        return NULL;
    }
    
    if (sem_init(clientSem, 0, 0) != 0) {
        free(clientSem);
        perror("Failed to initialize semaphore");
        return NULL;
    }
    
    // Store pointer to semaphore
    clienteConfig.sinalizarVerificaSala = clientSem;
    clienteConfig.jaSaiu = false;
    
    while (!clienteDesconectado && (bytesRecebidos = readSocket(socketCliente, buffer, BUF_SIZE)) > 0) {
        // Log received message
        // addClientSocket(clienteConfig.idCliente,socketCliente);
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
            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo,SalaID);
            if (strcmp(msgData.tipoJogo, "SIG") == 0) {
                salaAtualSIG = handleSinglePlayerFila(&clienteConfig, &serverConfig);
                if (!salaAtualSIG) {
                    const char* filaCheia = "FILA CHEIA SINGLEPLAYER";
                    send(socketCliente, filaCheia, strlen(filaCheia), 0);
                    clienteConfig.idCliente = atoi(msgData.idCliente);
                    clienteDesconectado = true;
                    break;
                }

                jogoADar = salaAtualSIG->jogo.jogo;
                nJogo = salaAtualSIG->jogo.idJogo;
                SalaID = salaAtualSIG->idSala;
            }
            if (strcmp(msgData.tipoJogo, "MUL_PRIMEIRO") == 0) {
                sem_post(&entradaPlayerMulPrimeiro);
                salaAtualMUL= &serverConfig.salaMultiplayer[0];
                pthread_barrier_wait(&barreiraComecaTodos);
                jogoADar = salaAtualMUL->jogo.jogo;
                nJogo = salaAtualMUL->jogo.idJogo;
                SalaID = salaAtualMUL->idSala;
            }

            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo,SalaID);

            bufferParaStructCliente(buffer, &clienteConfig);

            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                perror("Erro ao enviar mensagem para o cliente");
                break;
            }

            logQueEventoServidor(4, clienteConfig.idCliente, salaAtualSIG ? salaAtualSIG->idSala : 0);
            
            char bufferEnviarFinal[BUF_SIZE] = {0};
            snprintf(bufferEnviarFinal, BUF_SIZE, "Mensagem enviada: %s", buffer);
            logEventoServidor(bufferEnviarFinal);
            
            memset(buffer, 0, BUF_SIZE);
            continue;
        }

        if (strcmp(msgData.temJogo, "COM_JOGO") == 0) {
            if(strcmp(msgData.tipoJogo, "SIG") == 0) {
                if (!salaAtualSIG) {
                printf("Erro: Cliente sem sala atribuída\n");
                break;
                }

                atualizarClientConfig(&clienteConfig, &msgData);
                logQueEventoServidor(5, clienteConfig.idCliente, salaAtualSIG->idSala);

                char *logClienteEnviar = handleResolucaoJogo(&clienteConfig, salaAtualSIG);
                bool gameCompleted = verSeJogoAcabouEAtualizar(&clienteConfig, salaAtualSIG);

                memset(buffer, 0, BUF_SIZE);
                prepararRespostaJogo(buffer, &clienteConfig, logClienteEnviar);

                if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                    perror("Erro ao enviar mensagem para o cliente");
                    break;
                }

                logQueEventoServidor(6, clienteConfig.idCliente, salaAtualSIG->idSala);
                
                if (gameCompleted) {
                    break;
                }
            }
            
        }
    }
    if(salaAtualSIG) {
        pthread_mutex_lock(&salaAtualSIG->mutexSala);
        clienteConfig.jaSaiu = true;
        salaAtualSIG->jogadorAResolver = false;
        salaAtualSIG->clienteAtual.idCliente = -1;
        salaAtualSIG->nClientes = 0;
        pthread_mutex_unlock(&salaAtualSIG->mutexSala);
    }
    printf(COLOR_RED "Cliente %d saiu\n" COLOR_RESET, clienteConfig.idCliente);
    logQueEventoServidor(7, clienteConfig.idCliente, salaAtualSIG ? salaAtualSIG->idSala : 0);
    logQueEventoServidor(7, clienteConfig.idCliente, salaAtualSIG ? salaAtualSIG->idSala : 0);
}

struct filaClientesSinglePlayer *criarFila(struct ServidorConfig *serverConfig)
{
    struct filaClientesSinglePlayer *fila = (struct filaClientesSinglePlayer *)malloc(sizeof(struct filaClientesSinglePlayer));
    if (!fila)
    {
        perror("Erro ao alocar memoria para fila");
        return NULL;
    }
    fila->cliente = (struct ClienteConfig *)malloc(sizeof(struct ClienteConfig) * serverConfig->NUM_MAX_CLIENTES_FILA_SINGLEPLAYER);
    if (!fila->cliente)
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
        free(fila->cliente);
        free(fila);
        perror("Erro ao inicializar mutex");
        return NULL;
    }

    // Semaforo para os clientes notificarem as salas
    if (sem_init(&fila->customers, 0, 0) != 0)
    {
        pthread_mutex_destroy(&fila->mutex);
        free(fila->cliente);
        free(fila);
        perror("Erro ao inicializar semaforo");
        return NULL;
    }

    return fila;
}


bool estaFilaCheiaTrSafe(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);
    bool full = fila->tamanho == fila->capacidade;
    pthread_mutex_unlock(&fila->mutex);
    return full;
}

bool estaFilaVaziaTrSafe(struct filaClientesSinglePlayer *fila)
{
    pthread_mutex_lock(&fila->mutex);
    bool empty = fila->tamanho == 0;
    pthread_mutex_unlock(&fila->mutex);
    return empty;
}

bool enqueue(struct filaClientesSinglePlayer *fila, struct ClienteConfig cliente) {
    
    if (fila->tamanho >= fila->capacidade) {
        printf(COLOR_PURPLE"[Fila] Rejeitado cliente %d - fila cheia (tamanho: %d)\n"COLOR_RESET,
               cliente.idCliente, fila->tamanho);
        logQueEventoServidor(11, cliente.idCliente, 0);
        pthread_mutex_unlock(&fila->mutex);
        return false;
    }
    
    fila->rear = (fila->rear + 1) % fila->capacidade;
    fila->cliente[fila->rear] = cliente;  // This will copy the semaphore pointer
    fila->tamanho++;
    
    printf(COLOR_GREEN "[Fila] Cliente %d entrou na fila (posição: %d, tamanho: %d)\n"COLOR_RESET,
           cliente.idCliente, fila->rear, fila->tamanho);
    logQueEventoServidor(9, cliente.idCliente, 0);
    
    return true;
}

struct ClienteConfig dequeue(struct filaClientesSinglePlayer *fila)
{
    struct ClienteConfig clienteInvalido = {0};
    if (fila->tamanho == 0)
    {
        pthread_mutex_unlock(&fila->mutex);
        return clienteInvalido; // Queue is empty
    }

    // Save the client ID of the first element
    struct ClienteConfig cliente = fila->cliente[0];

    // Shift all the elements to the left
    for (int i = 1; i < fila->tamanho; i++)
    {
        fila->cliente[i - 1] = fila->cliente[i];
    }

    // Decrease the size of the queue
    fila->tamanho--;

    // Update rear pointer
    fila->rear = fila->tamanho - 1;

    printf(COLOR_RED "[Fila] Cliente %d removido da fila (tamanho restante: %d)\n"COLOR_RESET,
           cliente.idCliente, fila->tamanho);
           //passo 0 como nao importa para esta msg
    logQueEventoServidor(10, cliente.idCliente,0);
    return cliente;
}

void removerFilaPorID(struct filaClientesSinglePlayer *fila, int clientID) {
    pthread_mutex_lock(&fila->mutex);
    
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->cliente[i].idCliente == clientID) {
            // deslocar todos os elementos para a esquerda
            for (int j = i; j < fila->tamanho - 1; j++) {
                fila->cliente[j] = fila->cliente[j + 1];
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
        //acorda se tiver clientes na fila de espera
        sem_wait(&filaClientesSinglePlayer->customers);
        sem_wait(&acessoLugares);
        
        struct ClienteConfig cliente = dequeue(filaClientesSinglePlayer);
        if (cliente.idCliente == -1) {
            sem_post(&filaClientesSinglePlayer->customers);
            sem_post(&acessoLugares);
            continue;
        }
        
        sala->nClientes = 1;
        sala->jogadorAResolver = true;
        sala->clienteAtual = cliente;
        printf(COLOR_YELLOW "[Sala-%d] Atendendo cliente %d com jogo %d\n" COLOR_RESET,
        sala->idSala, sala->clienteAtual.idCliente, sala->jogo.idJogo);
        
        
        //para deixar a fila encher nao ser tao rapido
        sleep(1);
        sem_post(&acessoLugares);
        sem_post(cliente.sinalizarVerificaSala);
        
        
        bool clienteFinalizou = false;
        while (!clienteFinalizou) {
            if(cliente.jaSaiu){
                printf(COLOR_YELLOW "[Sala-%d] Cliente %d nao conseguiu resolver o jogo %d\n" COLOR_RESET,
                sala->idSala, sala->clienteAtual.idCliente, sala->jogo.idJogo);
            }
            if (!sala->jogadorAResolver) {
                sala->nClientes = 0;
                memset(&sala->clienteAtual, 0, sizeof(struct ClienteConfig));
                sala->clienteAtual.idCliente = -1;
                clienteFinalizou = true;
                break;
            }
        }
    }
    return NULL;
}
//5 jogadores a jogar ao mesmo tempo para ver sem acaba primeiro
void* SalaMultiplayerPrimeiro(void* arg) {
    struct SalaMultiplayer* sala = (struct SalaMultiplayer*) arg;

    enum GameState state = WAITING_PLAYERS;
    int naSala,esperandoEntrar = 0;
    bool temDeEsperar = false;
    pthread_barrier_init(&barreiraComecaTodos, NULL, 5);
    if(sem_init(&mutexSalaMultiplayer, 0, 1) || sem_init(&bloquearAcessoSala, 0, 0) || sem_init(&entradaPlayerMulPrimeiro, 0, 0)){
        perror("Erro ao inicializar semaforos");
        exit(1);
    }
    printf("[Sala-%d] Iniciada-Multiplayer\n", sala->idSala);
    while (1) {
        sem_wait(&entradaPlayerMulPrimeiro);
        if(sem_wait(&mutexSalaMultiplayer)){
            esperandoEntrar+=1;
            sem_post(&mutexSalaMultiplayer);
            sem_wait(&bloquearAcessoSala);
            esperandoEntrar-=1;
        }
        naSala+=1;
        temDeEsperar = (naSala==sala->clientesMax);
        if(esperandoEntrar && !temDeEsperar){
            sem_post(&bloquearAcessoSala);
        }
        else{
            sem_post(&mutexSalaMultiplayer);
        }
        switch(state) {
            case WAITING_PLAYERS:
                printf("[Sala-%d] Jogadores na sala, começando quando estiver cheio(%d/%d)\n", sala->idSala, naSala, sala->clientesMax);
                if(naSala == 5){
                    state = GAME_STARTING;
                }
                else{
                    break;
                }
                
            case GAME_STARTING:
                // Initialize game
                printf("[Sala-%d] A iniciar jogo\n", sala->idSala);
                state = GAME_RUNNING;
                break;
                
            case GAME_RUNNING:
                break;
                
            case GAME_ENDED:
                // Cleanup and possibly restart
                break;
        }
    }
    
    sem_destroy(&mutexSalaMultiplayer);
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
    SalaMultiplayerPrimeiro(sala);
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
        serverConfig->sala[i].clienteAtual.idCliente= -1;

        // Cada sala tem um jogo
        serverConfig->sala[i].jogo = jogosEsolucoes[i];

        if (pthread_mutex_init(&serverConfig->sala[i].mutexSala, NULL) != 0)
        {
            perror("Erro sala mutex");
            exit(1);
        }

        // Umasala é uma tread
        pthread_t threadSala;
        void *roomPtr = &serverConfig->sala[i];
        if (pthread_create(&threadSala, NULL, iniciarSalaSinglePlayer, roomPtr) != 0)
        {
            perror("Erro criar tarefa");
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

    int numeroTotalSalas = serverConfig->numeroJogos;
    serverConfig->salaMultiplayer = malloc(sizeof(struct SalaMultiplayer));
    for (int i = 0; i < 1; i++)
    {
        serverConfig->salaMultiplayer[i].idSala = numeroTotalSalas + i;
        serverConfig->salaMultiplayer[i].clientesMax = 5;
        serverConfig->salaMultiplayer[i].clienteMin = 5;
        serverConfig->salaMultiplayer[i].nClientes = 0;

        serverConfig->salaMultiplayer[i].jogoIniciado = false;
        serverConfig->salaMultiplayer[i].clienteAtualID = NULL;

        // Cada sala tem um jogo
        serverConfig->salaMultiplayer[i].jogo = jogosEsolucoes[i];
        

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

int main(int argc, char **argv) {
    struct ServidorConfig serverConfig = {0};
    sem_init(&acessoLugares,0,1);
    if (argc < 2) {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

    if (strcmp(argv[1], CONFIGFILE) != 0) {
        printf("Nome do ficheiro incorreto\n");
        return 1;
    }
    
    carregarConfigServidor(argv[1], &serverConfig);
    int numeroJogos = lerNumeroJogos(serverConfig.ficheiroJogosESolucoesCaminho);
    struct Jogo jogosEsolucoes[numeroJogos];
    serverConfig.numeroJogos = numeroJogos;
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho,jogosEsolucoes);
    construtorServer(&serverConfig,AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 5000, serverConfig.ficheiroJogosESolucoesCaminho);
    
    logQueEventoServidor(1, 0,0);
    filaClientesSinglePlayer = malloc(sizeof(struct filaClientesSinglePlayer));
    filaClientesSinglePlayer = criarFila(&serverConfig);
    
    iniciarServidorSocket(&serverConfig,jogosEsolucoes);
    return 0;
}