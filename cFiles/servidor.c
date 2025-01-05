#include "../headers/servidor.h"


struct filaClientesSinglePlayer *filaClientesSinglePlayer;

// globais
static int idCliente = 0;
int numeroJogosResolvidos=0;
int numeroClientesRejeitados=0;
int jogosEmResolucao=0;
int clientesAtuais=0;

pthread_mutex_t numeroJogosResolvidosMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t jogosEmResolucaoMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientesAtuaisMutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numeroClientesRejeitadosMutex=PTHREAD_MUTEX_INITIALIZER;

// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIDSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t aceitarCliente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verificaEstadoSala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAcabouJogo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verificarEstadoSalaMultiplayerFaster = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexIniciarJogoMULNORMAL=PTHREAD_MUTEX_INITIALIZER;

//cond
 pthread_cond_t condIniciarJogoMULNORMAL=PTHREAD_COND_INITIALIZER;

// semaforos
sem_t mutexSalaMultiplayerFaster;
sem_t mutexSalaMultiplayerNormal;
sem_t acessoLugares;
sem_t bloquearAcessoSala;
sem_t entradaPlayerMulPrimeiro;
sem_t capacidadeSalaMultiplayeFaster;
sem_t ultimoClienteSairSalaMultiplayerFaster;
sem_t jogoAcabouMultiplayerFaster;
sem_t iniciarJogoMul;
pthread_barrier_t barreiraComecaTodos;

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
    fprintf(file, " %s\n",message);
    fecharFicheiro(file);
    pthread_mutex_unlock(&mutexServidorLog);
}

// TODO METER LOGS QUANDO ENTRA E SAI DAS SALAS
void logQueEventoServidor(int numero, int clienteID,int salaID)
{
    //alocar memoria para mensagem e fazer especie de memset a zeros
    char mensagem[512]={0};
    // if (mensagem == NULL)
    // {
    //     perror("Erro ao alocar memoria para mensagem");
    //     exit(1);
    // }

    switch (numero)
    {
    case 1:
        logEventoServidor("Servidor comecou");
        // free(mensagem);
        break;
    case 2:
        logEventoServidor("Servidor parou");
        //free(mensagem);
        break;
    case 3:
        sprintf(mensagem, "[%s] Cliente-%d conectado",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 4:
        sprintf(mensagem, "[%s] Servidor enviou um jogo para o cliente-%d",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 5:
        sprintf(mensagem, "[%s] Servidor recebeu uma solucao do cliente-%d",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 6:
        sprintf(mensagem, "[%s] Servidor enviou uma solucao para o cliente-%d",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 7:
        sprintf(mensagem, "[%s] Cliente-%d desconectado",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 8:
        sprintf(mensagem, "[%s] Cliente-%d resolveu o jogo da sala-%d",getTempoHoraMinutoSegundoMs(), clienteID,salaID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 9:
        sprintf(mensagem, "[%s] Cliente-%d entrou na fila",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 10:
        sprintf(mensagem, "[%s] Cliente-%d removido da fila",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 11:
        pthread_mutex_lock(&numeroClientesRejeitadosMutex);
        numeroClientesRejeitados++;
        pthread_mutex_unlock(&numeroClientesRejeitadosMutex);
        sprintf(mensagem, "[%s] Cliente-%d rejeitado a entrar na fila",getTempoHoraMinutoSegundoMs(), clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 12:
        sprintf(mensagem, "[%s] Cliente-%d entrou na sala-%d",getTempoHoraMinutoSegundoMs(), clienteID,salaID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 13:
        sprintf(mensagem, "[%s] Cliente-%d não resolveu o jogo da sala-%d",getTempoHoraMinutoSegundoMs(), clienteID,salaID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 14:
        sprintf(mensagem,"[%s] Atendendo Cliente-%d na sala-%d",getTempoHoraMinutoSegundoMs(),clienteID,salaID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    case 15:
        sprintf(mensagem,"[%s] Cliente-%d venceu jogo multiplayer faster",getTempoHoraMinutoSegundoMs(),clienteID);
        logEventoServidor(mensagem);
        //free(mensagem);
        break;
    default:
        logEventoServidor("Evento desconhecido");
        //free(mensagem);
        break;
    }
}

// atualiza os valoresCorretos da Ultima Tentativa
char *atualizaValoresCorretos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas)
{
    // Aloca dinamicamente espaço para logClienteFinal
    char *logClienteFinal = calloc(2 * BUF_SIZE, sizeof(char));
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

    // *nTentativas = *nTentativas + 1;
    

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
    servidor->sala = calloc(servidor->numeroJogos, sizeof(struct SalaSinglePlayer));
    if (!servidor->sala) {
        perror("Erro ao alocar memória para salas");
        exit(1);
    }

    //colocar a 0 as posicoes de mem que temos
    // memset(servidor->sala, 0, servidor->numeroJogos * sizeof(struct SalaSinglePlayer));

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
        pthread_mutex_lock(&clientesAtuaisMutex);
        clientesAtuais++;
        pthread_mutex_unlock(&clientesAtuaisMutex);
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
        memset(args, 0, sizeof(struct ThreadCliente));
        
        args->socketCliente = socketCliente;
        args->server = server;

        // Criar thread para permitir clientes
        pthread_t thread;
        pthread_mutex_unlock(&aceitarCliente);
        
        if (pthread_create(&thread, NULL, criaClienteThread, args) != 0)
        {
            perror("Failed to create thread");
            free(args);
            
            close(socketCliente);
            continue;
        }
        // Liberta recursos da thread
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
//BarberShop problem com vários barbeiros FIFO-Customer
//Parte de sinalizar que acabou de ser atendido na funcao
//receberMensagemETratarServer 
// Função que dá handle a entrada de um cliente na fila e sua atribuição a uma sala
struct SalaSinglePlayer* handleSinglePlayerFila(struct ClienteConfig *cliente, struct ServidorConfig* serverConfig) {
    struct SalaSinglePlayer* salaEncontrada = NULL;
    //acesso aos lugares disponiveis
    sem_wait(&acessoLugares);
    printf("Cliente %d entrou na fila\n", cliente->idCliente);
    if (!enqueue(filaClientesSinglePlayer, *cliente)) {
        
        sem_post(&acessoLugares);
        return NULL;
    }
    //outro cliente pode verificar
    sem_post(&acessoLugares);
    
    //sinaliza a sala que tem um cliente
    sem_post(&filaClientesSinglePlayer->customers);
    //fica aqui à espera de ser chamado pela sala
    sem_wait(cliente->sinalizarVerificaSala);
    //parte de saber em que sala está
    //a parter de receber um haircut é a parte de
    //resolver o jogo
    //ver comentarios onde esta funcao retorna
    for (int i = 0; i < serverConfig->numeroJogos; i++) {
        if (serverConfig->sala[i].clienteAtual.idCliente == cliente->idCliente) {
            salaEncontrada = &serverConfig->sala[i];
            break;
        }
    }

    // sem_destroy(cliente->sinalizarVerificaSala);
    // free(cliente->sinalizarVerificaSala);
    
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
    strcpy(clienteConfig->jogoAtual.tempoInicio, getTempoHoraMinutoSegundoMs());
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

char *handleResolucaoJogoSIG(struct ClienteConfig *clienteConfig, struct SalaSinglePlayer *sala)
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
char *handleResolucaoJogoMUL(struct ClienteConfig *clienteConfig, struct SalaMultiplayer *sala)
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
        strcpy(cliente->jogoAtual.tempoFinal, getTempoHoraMinutoSegundoMs());

        sala->jogadorAResolver = false;
        sala->nClientes = 0;
        sala->clienteAtual.idCliente = -1;
        time_t tempoInicioConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoInicio);
        time_t tempoFinalConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoFinal);
        time_t tempoDemorado = difftime(tempoFinalConvertido, tempoInicioConvertido);
        
        printf("[Sala-%d] Cliente %d resolveu o jogo %d em %ld segundos!\n",
               sala->idSala, cliente->idCliente, sala->jogo.idJogo, tempoDemorado);
        //passo 0 como nao importa para esta msg
        logQueEventoServidor(8, cliente->idCliente, sala->idSala);
        pthread_mutex_lock(&numeroJogosResolvidosMutex);
        numeroJogosResolvidos++;
        pthread_mutex_unlock(&numeroJogosResolvidosMutex);
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

// de clienteConfig para buffer para mandar resposta
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
void adicionarClienteSalaMultiplayer(struct ServidorConfig *serverConfig, struct ClienteConfig cliente) {
    struct SalaMultiplayer *sala = &(serverConfig->salaMultiplayer[1]);
    for (int i = 0; i < serverConfig->numeroJogos; i++) {
        if (sala->nClientes < sala->clientesMax) {
            sala->clientes[sala->nClientes] = cliente;
            sala->nClientes++;
            break;
        }
    }
    if(sala->nClientes==sala->clientesMax){
        sem_post(&iniciarJogoMul);
    }
}
void adicionarClienteSalaMultiplayerFaster(struct ServidorConfig *serverConfig, struct ClienteConfig cliente) {
    struct SalaMultiplayer *sala = &(serverConfig->salaMultiplayer[0]);

    // tenta adicionar se nao for ordenado a esperar
    if(!sala->temDeEsperar) {
        // Find first empty spot
        for(int i = 0; i < sala->clientesMax; i++) {
            if(sala->clientes[i].idCliente == 0) { // Check for empty spot
                sala->clientes[i] = cliente;
                // printf("[Sala-%d] Cliente %d adicionado na posição %d\n", 
                //        sala->idSala, 
                //        cliente.idCliente, 
                //        i);
                       
                logQueEventoServidor(12, cliente.idCliente, sala->idSala);
                break;
            }
        }
    }
}
void limparAdicionarClienteSalaMultiplayerFaster(struct SalaMultiplayer *sala) {
    for(int i=0;i<sala->clientesMax;i++){
        memset(&sala->clientes[i], 0, sizeof(struct ClienteConfig));
    }
}
void receberMensagemETratarServer(char *buffer, int socketCliente,
                                 struct ClienteConfig clienteConfig,
                                 struct ServidorConfig serverConfig)
{
    struct SalaSinglePlayer *salaAtualSIG = NULL;
    struct SalaMultiplayer *salaAtualMULFaster= NULL;
    struct SalaMultiplayer *salaAtualMUL = NULL;
    char *jogoADar = "";
    int nJogo = -1;
    int bytesRecebidos;
    int SalaID = -1;
    bool clienteDesconectado = false;

    //sempre que um cliente novo entra tem o seu proprio semaforo
    sem_t *clientSem = malloc(sizeof(sem_t));
    if (!clientSem) {
        perror("Failed to allocate semaphore");
        return;
    }
    memset(clientSem, 0, sizeof(sem_t));
    if (sem_init(clientSem, 0, 0) != 0) {
        free(clientSem);
        perror("Failed to initialize semaphore");
        return;
    }
    
    // Apontar para o semáforo do cliente
    clienteConfig.sinalizarVerificaSala = clientSem;
    clienteConfig.socket = socketCliente;
    
    while (!clienteDesconectado && (bytesRecebidos = readSocket(socketCliente, buffer, BUF_SIZE)) > 0) {

        char bufferFinal[BUF_SIZE] = {0};
        snprintf(bufferFinal, BUF_SIZE, "[%s] Mensagem recebida: %s",getTempoHoraMinutoSegundoMs(), buffer);
        logEventoServidor(bufferFinal);

        struct FormatoMensagens msgData = parseMensagem(buffer);
        if (!validarMensagemVazia(&msgData)) {
            printf("Erro: Mensagem recebida com formato inválido\n");
            break;
        }

        // handle jogo inicial
        if (strcmp(msgData.temJogo, "SEM_JOGO") == 0) {
            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo,SalaID);
            if (strcmp(msgData.tipoJogo, "SIG") == 0) {
                //vai mandar jogo cliente posteriormente
                //continuacao haircut/resolucao do jogo em baixo jogo completo
                //se o cliente terminou é disponibilizada a msg
                //que o cliente resolveu senao
                salaAtualSIG = handleSinglePlayerFila(&clienteConfig, &serverConfig);
                if (!salaAtualSIG) {
                    const char* filaCheia = "FILA CHEIA SINGLEPLAYER";
                    writeSocket(clienteConfig.socket, filaCheia, strlen(filaCheia));
                    clienteConfig.idCliente = atoi(msgData.idCliente);
                    clienteDesconectado = true;
                    break;
                }

                jogoADar = salaAtualSIG->jogo.jogo;
                nJogo = salaAtualSIG->jogo.idJogo;
                SalaID = salaAtualSIG->idSala;
            }
            if (strcmp(msgData.tipoJogo, "MUL_FASTER") == 0) {
                sem_post(&entradaPlayerMulPrimeiro);
                //apenas 5 players entram
                sem_wait(&capacidadeSalaMultiplayeFaster);
                sem_wait(&mutexSalaMultiplayerFaster);
                adicionarClienteSalaMultiplayerFaster(&serverConfig,clienteConfig);
                sem_post(&mutexSalaMultiplayerFaster);
                salaAtualMULFaster = &serverConfig.salaMultiplayer[0];
                jogoADar = salaAtualMULFaster->jogo.jogo;
                nJogo = salaAtualMULFaster->jogo.idJogo;
                SalaID = salaAtualMULFaster->idSala;
                //espera todos para começar ao mesmo tempo - (roller coaster-espera todos embarcar)
                //nao havendo opcao de fazer unboard enquanto o carrinho nao fez o percurso
                //"Passengers cannot unboard until the car has invoked unload"
                //ou seja unload é a parte depois de alguem ganhar o jogo
                pthread_barrier_wait(&barreiraComecaTodos);
                //sleep por causa de prints
                sleep(1);
            }
            if(strcmp(msgData.tipoJogo,"MUL")==0){
                salaAtualMUL=&serverConfig.salaMultiplayer[1];
                jogoADar = salaAtualMUL->jogo.jogo;
                nJogo = salaAtualMUL->jogo.idJogo;
                SalaID = salaAtualMUL->idSala;
                salaAtualMUL->nClientes++;
            }

            updateClientConfig(&clienteConfig, &msgData, jogoADar, nJogo,SalaID);

            bufferParaStructCliente(buffer, &clienteConfig);
            if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                perror("Erro ao enviar mensagem para o cliente");
                break;
            }
            // if(strcmp(msgData.tipoJogo, "SIG") == 0){
            //     // logQueEventoServidor(4, clienteConfig.idCliente, salaAtualSIG ? salaAtualSIG->idSala : 0);
            // }
            // else{
            //     // logQueEventoServidor(4, clienteConfig.idCliente, salaAtualMUL ? salaAtualMUL->idSala : 0);
            // }
            
            char bufferEnviarFinal[BUF_SIZE] = {0};
            snprintf(bufferEnviarFinal, BUF_SIZE, "[%s] Mensagem enviada: %s",getTempoHoraMinutoSegundoMs(), buffer);
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
                // logQueEventoServidor(5, clienteConfig.idCliente, salaAtualSIG->idSala);

                char *logClienteEnviar = handleResolucaoJogoSIG(&clienteConfig, salaAtualSIG);
                
                bool gameCompleted = verSeJogoAcabouEAtualizar(&clienteConfig, salaAtualSIG);

                memset(buffer, 0, BUF_SIZE);
                prepararRespostaJogo(buffer, &clienteConfig, logClienteEnviar);

                if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                    perror("Erro ao enviar mensagem para o cliente");
                    break;
                }
                char bufferEnviarFinal[BUF_SIZE] = {0};
                snprintf(bufferEnviarFinal, BUF_SIZE, "[%s] Mensagem enviada: %s",getTempoHoraMinutoSegundoMs(), buffer);
                logEventoServidor(bufferEnviarFinal);
                logQueEventoServidor(6, clienteConfig.idCliente, salaAtualSIG->idSala);
                
                if (gameCompleted) {
                    break;
                }
            }
            if (strcmp(msgData.tipoJogo, "MUL_FASTER") == 0) {
                if (!salaAtualMULFaster) {
                    printf("Erro: Cliente sem sala atribuída\n");
                    break;
                }
                bool gameCompleted;
                atualizarClientConfig(&clienteConfig, &msgData);
                // logQueEventoServidor(5, clienteConfig.idCliente, salaAtualMUL->idSala);
                
                char *logClienteEnviar = handleResolucaoJogoMUL(&clienteConfig, salaAtualMULFaster);
                
                    if(salaAtualMULFaster->hasWinner){
                    continue;
                    }
                    sem_wait(&mutexSalaMultiplayerFaster);
                    gameCompleted = verSeJogoAcabouEAtualizarMultiplayerFaster(&clienteConfig, salaAtualMULFaster);
                    
                    
                    sem_post(&mutexSalaMultiplayerFaster);
                
                memset(buffer, 0, BUF_SIZE);
                prepararRespostaJogo(buffer, &clienteConfig, logClienteEnviar);
                
                if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                    perror("Erro ao enviar mensagem para o cliente");
                    break;
                }
                
                logQueEventoServidor(6, clienteConfig.idCliente, salaAtualMULFaster->idSala);
                if(gameCompleted){
                    sem_post(&salaAtualMULFaster->sinalizarVencedor);
                }
                
            }
            if(strcmp(msgData.tipoJogo, "MUL") == 0){
                bool gameCompleted=false;
                adicionarClienteSalaMultiplayer(&serverConfig,clienteConfig);
                atualizarClientConfig(&clienteConfig, &msgData);
                // logQueEventoServidor(5, clienteConfig.idCliente, salaAtualMUL->idSala);
                
                char *logClienteEnviar = handleResolucaoJogoMUL(&clienteConfig, salaAtualMUL);
                gameCompleted = verSeJogoAcabouEAtualizarMultiplayerFaster(&clienteConfig, salaAtualMUL);
                memset(buffer, 0, BUF_SIZE);
                prepararRespostaJogo(buffer, &clienteConfig, logClienteEnviar);
                
                if (writeSocket(socketCliente, buffer, BUF_SIZE) < 0) {
                    perror("Erro ao enviar mensagem para o cliente");
                    break;
                }
                
                logQueEventoServidor(6, clienteConfig.idCliente, salaAtualMUL->idSala);
                if(gameCompleted){
                    break;
                }
            }
            
        }
    }
    if(salaAtualSIG) {
        pthread_mutex_lock(&salaAtualSIG->mutexSala);
        sem_post(&salaAtualSIG->jogadorFinalizou);
        
        if(salaAtualSIG->jogadorAResolver){
            printf("Cliente %d não resolveu o jogo\n",clienteConfig.idCliente);
            logQueEventoServidor(13, clienteConfig.idCliente, salaAtualSIG->idSala);
        }
        
        salaAtualSIG->jogadorAResolver = false;
        salaAtualSIG->clienteAtual.idCliente = -1;
        salaAtualSIG->nClientes = 0;
        sem_wait(&salaAtualSIG->salaPronta);
        pthread_mutex_unlock(&salaAtualSIG->mutexSala);
        
    }
    if(salaAtualMULFaster) {
        //agora ja podem fazer unboard depois de jogo acabar
        sem_wait(&jogoAcabouMultiplayerFaster);
        salaAtualMULFaster->nClientes--;
        // printf("Numero de clientes na sala %d\n",salaAtualMUL->nClientes);
        //sinalizar que todos os clientes sairam
        if(salaAtualMULFaster->nClientes == 0){
            sem_post(&ultimoClienteSairSalaMultiplayerFaster);
        }
        //para termos unboard um a um
        sem_post(&jogoAcabouMultiplayerFaster);
    
    }
    if(salaAtualMUL){
        salaAtualMUL->nClientes--;
    }
    printf(COLOR_RED "Cliente %d saiu\n" COLOR_RESET, clienteConfig.idCliente);
    logQueEventoServidor(7, clienteConfig.idCliente, salaAtualSIG ? salaAtualSIG->idSala : 0);
    pthread_mutex_lock(&clientesAtuaisMutex);
    clientesAtuais--;
    pthread_mutex_unlock(&clientesAtuaisMutex);
    pthread_mutex_lock(&jogosEmResolucaoMutex);
    jogosEmResolucao--;
    pthread_mutex_unlock(&jogosEmResolucaoMutex);
}

struct filaClientesSinglePlayer *criarFila(struct ServidorConfig *serverConfig)
{
    struct filaClientesSinglePlayer *fila = (struct filaClientesSinglePlayer *)malloc(sizeof(struct filaClientesSinglePlayer));
    if (!fila)
    {
        perror("Erro ao alocar memoria para fila");
        return NULL;
    }
    memset(fila, 0, sizeof(struct filaClientesSinglePlayer));
    fila->cliente = (struct ClienteConfig *)calloc(serverConfig->NUM_MAX_CLIENTES_FILA_SINGLEPLAYER, sizeof(struct ClienteConfig));
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

bool enqueue(struct filaClientesSinglePlayer *fila, struct ClienteConfig cliente) {
    
    if (fila->tamanho >= fila->capacidade) {
        printf(COLOR_PURPLE"[Fila] Rejeitado cliente %d - fila cheia (tamanho: %d)\n"COLOR_RESET,
               cliente.idCliente, fila->tamanho);
        logQueEventoServidor(11, cliente.idCliente, 0);
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

//BarberShop problem com vários barbeiros FIFO-Barber
void* SalaSingleplayer(void* arg) {
    struct SalaSinglePlayer* sala = (struct SalaSinglePlayer*)arg;
    printf("[Sala-%d] Iniciada-Singleplayer\n", sala->idSala);
    sem_init(&sala->salaPronta,0,0);
    sem_init(&sala->jogadorFinalizou,0,0);
    while (1) {
        //acorda se tiver clientes na fila de espera
        sem_wait(&filaClientesSinglePlayer->customers);
        //sem para controlar o acesso aos lugares de espera
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
        logQueEventoServidor(14, cliente.idCliente, sala->idSala);
        pthread_mutex_lock(&jogosEmResolucaoMutex);
        jogosEmResolucao++;
        pthread_mutex_unlock(&jogosEmResolucaoMutex);
        //para deixar a fila encher nao ser tao rapido
        //visto que estou a usar tempo entre tentativas 0
        //posso tirar o sleep se estiver a usar mais tempo entre tentativas para deixar fila encher
        sleep(1);
        sem_post(&acessoLugares);
        //sinaliza que o cliente pode entrar
        sem_post(cliente.sinalizarVerificaSala);
        
        
        // bool clienteFinalizou = false;
        sem_wait(&sala->jogadorFinalizou);
                sala->nClientes = 0;
                memset(&sala->clienteAtual, 0, sizeof(struct ClienteConfig));
                sala->clienteAtual.idCliente = -1;
                // clienteFinalizou = true;
        sem_post(&sala->salaPronta);

    }
    return NULL;
}
bool verSeJogoAcabouEAtualizarMultiplayerFaster(struct ClienteConfig *cliente, struct SalaMultiplayer *sala) {   
    pthread_mutex_lock(&sala->winnerMutex);
    
    // If we already have a winner, return false
    if (sala->hasWinner) {
        pthread_mutex_unlock(&sala->winnerMutex);
        return true;
    }

    // Check if this client won
    if (verificaResolvido(cliente->jogoAtual.valoresCorretos, sala->jogo.solucao)) {
        cliente->jogoAtual.resolvido = 1;
        strcpy(cliente->jogoAtual.tempoFinal, getTempoHoraMinutoSegundoMs());

        sala->hasWinner = true;
        sala->winnerID = cliente->idCliente;

        time_t tempoInicioConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoInicio);
        time_t tempoFinalConvertido = converterTempoStringParaTimeT(cliente->jogoAtual.tempoFinal);
        time_t tempoDemorado = difftime(tempoFinalConvertido, tempoInicioConvertido);

        printf("[Sala-%d] Cliente %d resolveu o jogo %d em %ld segundos!\n",
               sala->idSala, cliente->idCliente, sala->jogo.idJogo, tempoDemorado);
        //passo 0 como nao importa para esta msg
        logQueEventoServidor(8, cliente->idCliente, sala->idSala);
        pthread_mutex_lock(&numeroJogosResolvidosMutex);
        numeroJogosResolvidos++;
        pthread_mutex_unlock(&numeroJogosResolvidosMutex);
        pthread_mutex_unlock(&sala->winnerMutex);
        return true;
    }

    pthread_mutex_unlock(&sala->winnerMutex);
    return false;
}
//5 jogadores a jogar ao mesmo tempo para ver quem acaba primeiro
void* SalaMultiplayerFaster(void* arg) {
    struct SalaMultiplayer* sala = (struct SalaMultiplayer*) arg;
    limparAdicionarClienteSalaMultiplayerFaster(sala);
    enum GameState state = WAITING_PLAYERS;
    char entrouSala[50];
    sala->temDeEsperar = false;
    sala->esperandoEntrar=0;
    pthread_barrier_init(&barreiraComecaTodos, NULL, 5);
    if(sem_init(&mutexSalaMultiplayerFaster, 0, 1)
     || sem_init(&bloquearAcessoSala, 0, 0)
      || sem_init(&entradaPlayerMulPrimeiro, 0, 0)
       || sem_init(&sala->sinalizarVencedor,0,0)
        || sem_init(&capacidadeSalaMultiplayeFaster,0,5)
        || sem_init(&ultimoClienteSairSalaMultiplayerFaster,0,0)
        || sem_init(&jogoAcabouMultiplayerFaster,0,1)){
        perror("Erro ao inicializar semaforos");
        exit(1);
    }
    sala->hasWinner = false;
    sala->winnerID = -1;
    if (pthread_mutex_init(&sala->winnerMutex, NULL) != 0)
        {
            perror("Erro sala mutex");
            exit(1);
        }
    printf("[Sala-%d] Iniciada-Multiplayer\n", sala->idSala);
    while (1) {
        sem_wait(&entradaPlayerMulPrimeiro);
        
        // Entry protocol (similar to sushi bar)
        sem_wait(&mutexSalaMultiplayerFaster);
        // check para ver se houve clientes a sair
        if (sala->temDeEsperar) {
            sala->esperandoEntrar++;
            sem_post(&mutexSalaMultiplayerFaster);
            sem_wait(&bloquearAcessoSala);  // When we resume, we hold the mutex
            sala->esperandoEntrar--;
        }
        
        sala->nClientes++;
        pthread_mutex_lock(&jogosEmResolucaoMutex);
        jogosEmResolucao++;
        pthread_mutex_unlock(&jogosEmResolucaoMutex);
        sala->temDeEsperar = (sala->nClientes == sala->clientesMax);
        
        // Like sushi bar: if there are waiting threads and room isn't full
        if (sala->esperandoEntrar > 0 && !sala->temDeEsperar) {
            sem_post(&bloquearAcessoSala);  // Transfer the mutex
        } else {
            sem_post(&mutexSalaMultiplayerFaster);
        }
        //comer sushi
        switch(state) {
            //estado inicial entrada de players(roller coaster-load)
            case WAITING_PLAYERS:
               
                printf("[Sala-%d] Jogadores na sala, começando quando estiver cheio(%d/%d)\n", sala->idSala, sala->nClientes, sala->clientesMax);
                int jogadoresFaltantes = sala->clientesMax - sala->nClientes;
                for(int i=0;i<sala->nClientes;i++){
                    sprintf(entrouSala,"ENTROU_FASTER|%d",jogadoresFaltantes);
                    writeSocket(sala->clientes[i].socket,entrouSala,strlen(entrouSala));
                }
                if(sala->nClientes == 5){
                    state = GAME_STARTING;
                }
                else{
                    break;
                }
            //estado de preparação para o jogo
            case GAME_STARTING:
                // Initialize game
                printf("[Sala-%d] A iniciar jogo\n", sala->idSala);
                sem_wait(&jogoAcabouMultiplayerFaster);
                state = GAME_RUNNING;
            //estado de execução do jogo(roller coaster-run)
            case GAME_RUNNING:
                // verSeJogoAcabouEAtualizarMultiplayerFaster a verificar se alguem acabou

                // Broadcast quem ganhou para todos os clientes
                sem_wait(&sala->sinalizarVencedor);
                char winnerMsg[32];
                sprintf(winnerMsg, "WINNER|%d", sala->winnerID);
                printf("[Sala-%d] Vencedor ID: %d\n", sala->idSala, sala->winnerID);
                logQueEventoServidor(15, sala->winnerID, sala->idSala);
                for (int i = 0; i < sala->clientesMax; i++) {
                    if (writeSocket(sala->clientes[i].socket, winnerMsg, strlen(winnerMsg)) < 0) {
                        perror("Falha ao mandar mensagem de quem ganhou, um ou mais clientes sairam,perderam ligacao ou desistiram o que nao é permitido");
                    }
                    char bufferEnviarFinal[BUF_SIZE] = {0};
                    snprintf(bufferEnviarFinal, BUF_SIZE, "[%s] Mensagem enviada: %s",getTempoHoraMinutoSegundoMs(), winnerMsg);
                    logEventoServidor(bufferEnviarFinal);
                }
                sala->hasWinner = true;
                // Reset game state
                state=GAME_ENDED;
            
            //estado final - (roller coaster-unload)    
            case GAME_ENDED:
            //agora os jogadores podem sair
            sem_post(&jogoAcabouMultiplayerFaster);
            //aguarda todos os jogadores desembarcar
            sem_wait(&ultimoClienteSairSalaMultiplayerFaster);
            sem_wait(&mutexSalaMultiplayerFaster);
            
                printf("[Sala-%d] Jogo terminado e ultimo cliente saiu\n", sala->idSala);
                sala->jogoIniciado = false;
                sala->hasWinner = false;
                sala->winnerID = -1;
                state=WAITING_PLAYERS;
                sala->temDeEsperar = false;
                // limpa a sala
                limparAdicionarClienteSalaMultiplayerFaster(sala);
                if(sala->esperandoEntrar > 0){
                    sem_post(&bloquearAcessoSala);
                }
                else{
                    sem_post(&mutexSalaMultiplayerFaster);
                }
            //libertar lugares
            for(int i=0;i<sala->clientesMax;i++){
                sem_post(&capacidadeSalaMultiplayeFaster);
            }
            break;
        }
    }
    
    sem_destroy(&mutexSalaMultiplayerFaster);
    pthread_mutex_destroy(&sala->winnerMutex);
    return NULL;
}
void* SalaMultiplayer(void* arg) {
    struct SalaMultiplayer* sala = (struct SalaMultiplayer*) arg;
    enum GameState state = WAITING_PLAYERS;
    printf("[Sala-%d] Iniciada-Multiplayer\n", sala->idSala);
    
    // Lock para proteger o acesso à condição
    // pthread_mutex_lock(&mutexIniciarJogoMULNORMAL);
    if(sem_init(&iniciarJogoMul,0,0) || sem_init(&mutexSalaMultiplayerNormal, 0,1) != 0){
        perror("Erro ao inicializar mutex");
        exit(1);
    }

    while (1) {
        switch(state){
            case WAITING_PLAYERS:
                //espera 4 jogadores
                sem_wait(&iniciarJogoMul);
                printf("[Sala-%d] Jogadores na sala, começando quando estiver cheio(%d/%d)\n", sala->idSala, sala->nClientes, sala->clientesMax);
                if(sala->nClientes == 4){
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
                // verSeJogoAcabouEAtualizarMultiplayerFaster a verificar se alguem acabou
                // Broadcast quem ganhou para todos os clientes
                // Reset game state
                state = GAME_ENDED;
                break;
            case GAME_ENDED:
                printf("[Sala-%d] Jogo terminado\n", sala->idSala);
                state = WAITING_PLAYERS;
                break;
        }
    }
    
    return NULL;
}
void *iniciarSalaSinglePlayer(void *arg)
{
    struct SalaSinglePlayer *sala = (struct SalaSinglePlayer *)arg;
    SalaSingleplayer(sala);
    return NULL;
}
void *iniciarSalaMultiplayerFaster(void *arg)
{
    struct SalaMultiplayer *sala = (struct SalaMultiplayer *)arg;
    SalaMultiplayerFaster(sala);
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
    serverConfig->salaMultiplayer = calloc(2, sizeof(struct SalaMultiplayer));
    if(serverConfig->salaMultiplayer == NULL){
        perror("Erro ao alocar memoria para sala multiplayer");
        exit(1);
    }
    for (int i = 0; i < 1; i++)
    {
        serverConfig->salaMultiplayer[i].idSala = numeroTotalSalas + i;
        serverConfig->salaMultiplayer[i].clientesMax = 5;
        serverConfig->salaMultiplayer[i].clienteMin = 5;
        serverConfig->salaMultiplayer[i].nClientes = 0;

        serverConfig->salaMultiplayer[i].jogoIniciado = false;
        serverConfig->salaMultiplayer[i].clienteAtualID = NULL;
        serverConfig->salaMultiplayer[i].hasWinner = false;
        // Cada sala tem um jogo
        serverConfig->salaMultiplayer[i].jogo = jogosEsolucoes[i];
        serverConfig->salaMultiplayer[i].clientes = malloc(
        serverConfig->salaMultiplayer[i].clientesMax * sizeof(struct ClienteConfig));
        memset(serverConfig->salaMultiplayer[i].clientes, 0, serverConfig->salaMultiplayer[i].clientesMax * sizeof(struct ClienteConfig));
        // Umasala é uma tread
        pthread_t threadSala;
        void *roomPtr = &serverConfig->salaMultiplayer[i];
        if (pthread_create(&threadSala, NULL, iniciarSalaMultiplayerFaster, roomPtr) != 0)
        {
            perror("Failed to create barber thread");
            exit(1);
        }
        pthread_detach(threadSala);
    }

}
void* informacoesServidor(void* arg) {
    struct ServidorConfig* serverConfig = (struct ServidorConfig*)arg;
    sleep(5);
    while(1) {
        printf("\n[Sistema] Número de clientes atuais: %d\n", clientesAtuais);
        printf("[Sistema] Número de jogos resolvidos: %d\n", numeroJogosResolvidos);
        printf("[Sistema] Número de jogadores rejeitados: %d\n", numeroClientesRejeitados);
        sleep(10);
    }
    return NULL;
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
    pthread_t threadInformacoesServidor;
    pthread_create(&threadInformacoesServidor, NULL, informacoesServidor, &serverConfig);
    carregarConfigServidor(argv[1], &serverConfig);
    int numeroJogos = lerNumeroJogos(serverConfig.ficheiroJogosESolucoesCaminho);
    struct Jogo jogosEsolucoes[numeroJogos];
    serverConfig.numeroJogos = numeroJogos;
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho,jogosEsolucoes);
    construtorServer(&serverConfig,AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 5000, serverConfig.ficheiroJogosESolucoesCaminho);
    
    logQueEventoServidor(1, 0,0);
    filaClientesSinglePlayer = calloc(1, sizeof(struct filaClientesSinglePlayer));
    filaClientesSinglePlayer = criarFila(&serverConfig);
    
    iniciarServidorSocket(&serverConfig,jogosEsolucoes);
    free(filaClientesSinglePlayer);
    return 0;
}