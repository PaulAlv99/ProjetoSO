#include "../headers/cliente.h"
// Aceita cliente1.conf até clienteN.conf
// TODO locks para escrever no stdout
char *padrao = "./configs/cliente";

#define LINE_SIZE 16
// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSTDOUT = PTHREAD_MUTEX_INITIALIZER;

sem_t aguardaChegarClientes;
sem_t garfos[9];
sem_t mutexFilosofos;
enum Estados
{
    PENSANDO,
    COM_FOME,
    COMENDO
};
enum Estados estadosFilosofos[9];



bool posicoesSaoAdjacentes(int i,int j){
    if(i == 0){
        if(j == 1 || j == 8){
            return true;
        }
    }
    if(i == 1){
        if(j == 0 || j == 2){
            return true;
        }
    }
    if(i == 2){
        if(j == 1 || j == 3){
            return true;
        }
    }
    if(i == 3){
        if(j == 2 || j == 4){
            return true;
        }
    }
    if(i == 4){
        if(j == 3 || j == 5){
            return true;
        }
    }
    if(i == 5){
        if(j == 4 || j == 6){
            return true;
        }
    }
    if(i == 6){
        if(j == 5 || j == 7){
            return true;
        }
    }
    if(i == 7){
        if(j == 6 || j == 8){
            return true;
        }
    }
    if(i == 8){
        if(j == 7 || j == 0){
            return true;
        }
    }
    return false;
}
int left(int i)
{
    return i;
}
int right(int i)
{
    return (i + 1) % 4;
}
void get_fork(int i){
    sem_wait(&mutexFilosofos);
    estadosFilosofos[i] = COM_FOME;
    testar(i);
    sem_post(&mutexFilosofos);
    sem_wait(&garfos[i]);
}

void put_fork(int i){
    sem_wait(&mutexFilosofos);
    estadosFilosofos[i] = PENSANDO;
    testar(right(i));
    testar(left(i));
    sem_post(&mutexFilosofos);
}
void testar(int i){
    if (estadosFilosofos[i] == COM_FOME && estadosFilosofos[left(i)] != COMENDO && estadosFilosofos[right(i)] != COMENDO){
        estadosFilosofos[i] = COMENDO;
        sem_post(&garfos[i]);
    }
}
// void filosofo(int i){
//     while (1){
//         pensar();
// pegar em duas regioes
//         get_fork(i);
// resolver as duas regioes
//         comer();
//         put_fork(i);
//     }
// }

void carregarConfigCliente(char *nomeFicheiro, struct ClienteConfig *clienteConfig) {
    FILE *config = abrirFicheiroRead(nomeFicheiro);
    if (config == NULL) {
        fprintf(stderr, "Erro: Falha ao abrir o ficheiro de configuração.\n");
        exit(1);
    }

    char buffer[BUF_SIZE];
    int linhaAtual = 0;
    
    while (fgets(buffer, BUF_SIZE, config) != NULL) {
        char *valor = strtok(buffer, "\n");
        if (valor == NULL) {
            perror("Erro: Linha vazia encontrada.\n");
            fecharFicheiro(config);
            exit(1);
        }

        switch(linhaAtual) {
            case 0: // tipoJogo
                strncpy(clienteConfig->tipoJogo, valor, INFO_SIZE);
                break;
                
            case 1: // tipoResolucao
                strncpy(clienteConfig->tipoResolucao, valor, INFO_SIZE);
                break;
                
            case 2: // ipServidor
                strncpy(clienteConfig->ipServidor, valor, IP_SIZE - 1);
                break;
                
            case 3: // porta
                clienteConfig->porta = atoi(valor);
                break;
            case 4: // numJogadoresASimular
				clienteConfig->numJogadoresASimular = atoi(valor);
				break;
			case 5: // tempoEntreTentativas
				clienteConfig->tempoEntreTentativas = atoi(valor);
				break;
            default:
                perror("Erro ao ler ficheiro de configuração.\n");
                fecharFicheiro(config);
                exit(1);
        }
        
        linhaAtual++;
    }

    // Verifica se todas as configurações necessárias foram lidas
    if (linhaAtual < 4) {
        perror("Erro: Configuração incompleta\n");
        fecharFicheiro(config);
        exit(1);
    }

    fecharFicheiro(config);
}

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
	printf("\n");
}

void logEventoCliente(const char *message, struct ClienteConfig *clienteConfig)
{

	// modo append
	char *str = "logs/clienteLog.txt";
	pthread_mutex_lock(&mutexClienteLog);
	FILE *file = fopen(str, "a");
	if (file == NULL)
	{
		perror("Erro ao abrir o ficheiro de log");
		pthread_mutex_unlock(&mutexClienteLog);
		return;
	}
	fprintf(file, "[Cliente ID: %u] %s\n", clienteConfig->idCliente, message);

	fclose(file);
	pthread_mutex_unlock(&mutexClienteLog);
}

void logQueEventoCliente(int numero, struct ClienteConfig clienteConfig)
{
	switch (numero)
	{
	case 1:
		logEventoCliente("Cliente comecou o programa", &clienteConfig);
		break;
	case 3:
		logEventoCliente("Cliente conectou-se ao servidor", &clienteConfig);
		break;
	case 4:
		logEventoCliente("Cliente enviou uma mensagem ao servidor", &clienteConfig);
		break;
	case 5:
		logEventoCliente("Cliente recebeu uma resposta do servidor", &clienteConfig);
		break;
	case 6:
		logEventoCliente("Cliente desconectou-se do servidor", &clienteConfig);
		break;
	case 7:
		logEventoCliente("[ERRO]", &clienteConfig);
		break;
	case 8:
		logEventoCliente("Cliente resolveu jogo",&clienteConfig);
		break;
    case 9:
        logEventoCliente("Fila singleplayer cheia, jogador saiu",&clienteConfig);
        break;
    case 10:
        logEventoCliente("Cliente desistiu de resolver o jogo",&clienteConfig);
        break;
	default:
		logEventoCliente("Evento desconhecido", &clienteConfig);
		break;
	}
}

void construtorCliente(int dominio, unsigned int porta, __u_long interface, struct ClienteConfig *clienteConfig)
{
	strcpy(clienteConfig->TemJogo, "SEM_JOGO");
	clienteConfig->jogoAtual.resolvido = false;
	clienteConfig->jogoAtual.numeroTentativas = 0;
	clienteConfig->jogoAtual.idJogo = 0;
    clienteConfig->idSala = -1;
	size_t tamanhoStringJogo = strlen(clienteConfig->jogoAtual.jogo);
	memset(clienteConfig->jogoAtual.jogo + tamanhoStringJogo, '0', NUMEROS_NO_JOGO);
	*(clienteConfig->jogoAtual.jogo + NUMEROS_NO_JOGO + 1) = '\0';

	size_t tamanhoStringValoresCorretos = strlen(clienteConfig->jogoAtual.valoresCorretos);
	memset(clienteConfig->jogoAtual.valoresCorretos + tamanhoStringValoresCorretos, '0', NUMEROS_NO_JOGO);
	*(clienteConfig->jogoAtual.valoresCorretos + NUMEROS_NO_JOGO + 1) = '\0';

	strcpy(clienteConfig->jogoAtual.tempoInicio, "0");
	strcpy(clienteConfig->jogoAtual.tempoFinal, "0");
	clienteConfig->dominio = dominio;
	clienteConfig->porta = porta;
	clienteConfig->interface = interface;
}

void iniciarClienteSocket(struct ClienteConfig *clienteConfig)
{
	clienteConfig->socket = socket(clienteConfig->dominio, SOCK_STREAM, 0);
	if (clienteConfig->socket == -1)
	{
		perror("Erro ao criar socket");
		exit(1);
	}

	struct sockaddr_in enderecoServidor;
	enderecoServidor.sin_family = clienteConfig->dominio;
	enderecoServidor.sin_port = htons(clienteConfig->porta);

	if (inet_pton(clienteConfig->dominio, clienteConfig->ipServidor, &enderecoServidor.sin_addr) <= 0)
	{
		perror("Erro ao converter IP do servidor");
		close(clienteConfig->socket);
		exit(1);
	}
	int umaVez = 1;
    logQueEventoCliente(1, *clienteConfig);
	while (connect(clienteConfig->socket, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) == -1)
	{
		if (umaVez)
		{
			perror("Erro ao conectar ao servidor");
			printf("Tentando conectar ao servidor...\n");
			umaVez = 0;
		}
	}
    
	char* mandaID="MANDA_ID";

	if(writeSocket(clienteConfig->socket, mandaID, strlen(mandaID)) < 0){
		perror("Erro ao enviar ID");
		logQueEventoCliente(7, *clienteConfig);
		return;
	}
	char recebeIDCliente[BUF_SIZE] = {0};
    while(readSocket(clienteConfig->socket, recebeIDCliente, BUF_SIZE) < 0){
        
    }
    if(strstr(recebeIDCliente, "|") != NULL){
            clienteConfig->idCliente = atoi(strtok(recebeIDCliente, "|"));
            
    }
	// readSocket(clienteConfig->socket, recebeIDCliente, BUF_SIZE);
	// cliente recebe id do servidor
	mandarETratarMSG(clienteConfig);
	close(clienteConfig->socket);
}
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[])
{
	for (int i = 0; i < strlen(tentativaAtual); i++)
	{
		if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
		{
			char numero = tentativaAtual[i];
			int numeroInt = (int)(numero);
			int novoNumero = numeroInt + 1;
			char novoNumeroChar = (char)(novoNumero);
			tentativaAtual[i] = novoNumeroChar;
			break;
		}
		else if (tentativaAtual[i] == '0')
		{
			tentativaAtual[i] = '1';
			break;
		}
	}
}

// Atualiza a tentativaAtual
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[])
{
	for (int i = 0; i < strlen(tentativaAtual); i++)
	{
		if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
		{
			char numero = tentativaAtual[i];
			int numeroInt = (int)(numero);
			int novoNumero = numeroInt + 1;
			char novoNumeroChar = (char)(novoNumero);
			tentativaAtual[i] = novoNumeroChar;
		}
		else if (tentativaAtual[i] == '0')
		{
			tentativaAtual[i] = '1';
		}
	}
}


// Message formatting and parsing functions
void formatarMensagemJogo(char *buffer, const struct ClienteConfig *clienteConfig) {
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

bool parseMensagemJogo(const char *buffer, struct ClienteConfig *clienteConfig) {
    char *temp = strdup(buffer);
    if (!temp) return false;

    char *idCliente = strtok(temp, "|");
    char *tipoJogo = strtok(NULL, "|");
    char *tipoResolucao = strtok(NULL, "|");
    char *temJogo = strtok(NULL, "|");
    char *idJogo = strtok(NULL, "|");
    char *idSala = strtok(NULL, "|");
    char *jogo = strtok(NULL, "|");
    char *valoresCorretos = strtok(NULL, "|");
    char *tempoInicio = strtok(NULL, "|");
    char *tempoFinal = strtok(NULL, "|");
    char *resolvido = strtok(NULL, "|");
    char *numeroTentativas = strtok(NULL, "|");
    // char *logCliente = strtok(NULL, "|");

    bool sucesso = (idCliente && tipoJogo && tipoResolucao && temJogo && 
                   idJogo && idSala && valoresCorretos && tempoInicio && 
                   tempoFinal && resolvido && numeroTentativas);

    if (sucesso) {
        clienteConfig->idCliente = atoi(idCliente);
        strcpy(clienteConfig->TemJogo, temJogo);
        clienteConfig->jogoAtual.idJogo = atoi(idJogo);
        clienteConfig->idSala = atoi(idSala);
        strcpy(clienteConfig->jogoAtual.jogo, jogo);
        strcpy(clienteConfig->jogoAtual.valoresCorretos, valoresCorretos);
        strcpy(clienteConfig->jogoAtual.tempoInicio, tempoInicio);
        strcpy(clienteConfig->jogoAtual.tempoFinal, tempoFinal);
        clienteConfig->jogoAtual.resolvido = atoi(resolvido);
        clienteConfig->jogoAtual.numeroTentativas = atoi(numeroTentativas);
        
        // if (logCliente) {
        //     logEventoCliente(logCliente, clienteConfig);
        // }
    }

    free(temp);
    return sucesso;
}

bool enviarPedidoJogo(struct ClienteConfig *clienteConfig) {
    char buffer[BUF_SIZE] = {0};
    formatarMensagemJogo(buffer, clienteConfig);
    
    if (writeSocket(clienteConfig->socket, buffer, BUF_SIZE) < 0) {
        perror("Erro ao enviar mensagem para o servidor");
        logQueEventoCliente(7, *clienteConfig);
        return false;
    }
    char *log = calloc(2*BUF_SIZE, sizeof(char));
    if(log == NULL){
        perror("Erro ao alocar memoria para log");
        exit(1);
    }
    sprintf(log, "[%s] Mensagem enviada: %s",getTempoHoraMinutoSegundoMs(), buffer);
    logEventoCliente(log, clienteConfig);
    free(log);
    return true;
}

bool enviarTentativa(struct ClienteConfig *clienteConfig) {
    char buffer[BUF_SIZE] = {0};
    formatarMensagemJogo(buffer, clienteConfig);

    if (writeSocket(clienteConfig->socket, buffer, BUF_SIZE) < 0) {
        perror("Erro ao enviar tentativa");
        logQueEventoCliente(7, *clienteConfig);
        return false;
    }

    char *bufferLog = calloc(2 * BUF_SIZE, sizeof(char));
    if(bufferLog == NULL){
        perror("Erro ao alocar memoria para log");
        exit(1);
    }
    sprintf(bufferLog, "[%s] Mensagem enviada: %s",getTempoHoraMinutoSegundoMs(), buffer);
    logEventoCliente(bufferLog, clienteConfig);
    free(bufferLog);
    
    
    return true;
}

void atualizarTentativa(struct ClienteConfig *clienteConfig) {
    if (strcmp(clienteConfig->tipoResolucao, "COMPLET") == 0) {
        tentarSolucaoCompleta(clienteConfig->jogoAtual.jogo, 
                            clienteConfig->jogoAtual.valoresCorretos);
                            clienteConfig->jogoAtual.numeroTentativas++;
    }
    else if (strcmp(clienteConfig->tipoResolucao, "PARCIAL") == 0) {
        tentarSolucaoParcial(clienteConfig->jogoAtual.jogo, 
                            clienteConfig->jogoAtual.valoresCorretos);
                            clienteConfig->jogoAtual.numeroTentativas++;
    }
}

bool processarEstadoJogo(struct ClienteConfig *clienteConfig) {
    pthread_mutex_lock(&mutexSTDOUT);
    printf("Cliente ID:%d\n", clienteConfig->idCliente);
    printf("Sala ID:%d\n", clienteConfig->idSala);
    printf("\nTentativa %d:\n\n", clienteConfig->jogoAtual.numeroTentativas);
    imprimirTabuleiro(clienteConfig->jogoAtual.jogo);
    pthread_mutex_unlock(&mutexSTDOUT);

    // Realizar tentativa
    atualizarTentativa(clienteConfig);

    // Aguardar tempo entre tentativas
    //mudado na config do jogador
    struct timespec tempo = {
        .tv_sec = clienteConfig->tempoEntreTentativas / 1000,
        .tv_nsec = (clienteConfig->tempoEntreTentativas % 1000) * 1000000
    };
    nanosleep(&tempo, NULL);

    return enviarTentativa(clienteConfig);
}

void imprimirResultadoFinal(struct ClienteConfig *clienteConfig) {
    pthread_mutex_lock(&mutexSTDOUT);
    printf("Cliente ID:%d\n", clienteConfig->idCliente);
    printf("Tentativa: %d\n\n",clienteConfig->jogoAtual.numeroTentativas);
    imprimirTabuleiro(clienteConfig->jogoAtual.jogo);
    printf("Jogo resolvido!\n");
    printf("Resolvido em %d tentativas\n", 
           clienteConfig->jogoAtual.numeroTentativas);
    printf("Hora de inicio: %s\n", clienteConfig->jogoAtual.tempoInicio);
    printf("Hora de fim: %s\n", clienteConfig->jogoAtual.tempoFinal);
    pthread_mutex_unlock(&mutexSTDOUT);
}

void imprimirEstadoInicial(struct ClienteConfig *clienteConfig) {
    pthread_mutex_lock(&mutexSTDOUT);
    printf("Cliente ID:%d\n", clienteConfig->idCliente);
    printf("Iniciando tentativa de solução...\n");
    printf("Jogo Inicial:\n\n");
    printf("Hora de inicio: %s\n\n", clienteConfig->jogoAtual.tempoInicio);
    imprimirTabuleiro(clienteConfig->jogoAtual.jogo);
    pthread_mutex_unlock(&mutexSTDOUT);
}

//Funcao principal para enviar e receber mensagens
void mandarETratarMSG(struct ClienteConfig *clienteConfig) 
{
    char buffer[BUF_SIZE];
    ssize_t bytesRead;
    int jogadoresEmFalta;
    // Mandar pedido inicial de jogo
    if (!enviarPedidoJogo(clienteConfig)) {
        return;
    }
    
    //Parte principal do cliente
    while ((bytesRead = readSocket(clienteConfig->socket, buffer, BUF_SIZE)) > 0) {
        //em qualquer momento o cliente pode desistir de resolver 2% de probabilidade
        
        buffer[bytesRead] = '\0';
        //Para jogos singleplayer se estiver cheio dá break ou seja sai logo
        if (strcmp(buffer, "FILA CHEIA SINGLEPLAYER") == 0) {
            pthread_mutex_lock(&mutexSTDOUT);
            printf("Cliente ID:%d\n", clienteConfig->idCliente);
            printf("Fila singleplayer está cheia\n");
            pthread_mutex_unlock(&mutexSTDOUT);
            logQueEventoCliente(9, *clienteConfig);
            break;
        }
        //Para jogos multiplayer faster cada cliente recebe a mensagem
        //de quem ganhou, tendo em consideração o seu próprio id sabem se 
        //ganhou ou perdeu depois disso saem da sala e do servidor
        if (strcmp(buffer, "WINNER|") > 0) {
                int winnerID;
                sscanf(buffer, "WINNER|%d", &winnerID);
                
                pthread_mutex_lock(&mutexSTDOUT);
                if (winnerID == clienteConfig->idCliente) {
                    printf("Cliente ID:%d\n", clienteConfig->idCliente);
                    printf("\nVocê venceu o jogo!\n\n");
                    pthread_mutex_unlock(&mutexSTDOUT);
                    break;
                } else {
                    printf("Cliente ID:%d\n", clienteConfig->idCliente);
                    printf("\nVocê perdeu o jogo!\n\n");
                    pthread_mutex_unlock(&mutexSTDOUT);
                    break;
                }            
        }
        if(strcmp(buffer,"JOGO_CANCELADO") > 0){
            pthread_mutex_lock(&mutexSTDOUT);
            printf("Cliente ID: %d\n",clienteConfig->idCliente);
            printf("Jogo multiplayer faster cancelado\n");
            pthread_mutex_unlock(&mutexSTDOUT);
        }
        //servidor manda mensagem de quando entra um player na sala
        //multiplayer faster sempre com quantos players faltam entrar

        if(strcmp(buffer,"ENTROU_FASTER|") > 0){
            pthread_mutex_lock(&mutexSTDOUT);
            sscanf(buffer,"ENTROU_FASTER|%d",&jogadoresEmFalta);
            
            printf("\nCliente ID: %d\n",clienteConfig->idCliente);
            if(jogadoresEmFalta == 0){
                printf("Começando jogo...\n\n");
            }
            else{
                printf("Aguardando restantes %d jogadores\n\n",jogadoresEmFalta);
            }
            
            pthread_mutex_unlock(&mutexSTDOUT);
        }
        
        //transformar a mensagem recebida em struct clienteConfig
        if (!parseMensagemJogo(buffer, clienteConfig)) {
            continue;
        }
        
        //alocar memória para o log
        char *bufferLog = calloc(2 * BUF_SIZE, sizeof(char));
        if(bufferLog == NULL){
            perror("Erro ao alocar memoria para mensagem recebida");
            exit(1);
        }
        sprintf(bufferLog, "[%s] Mensagem recebida: %s",getTempoHoraMinutoSegundoMs(), buffer);
        logEventoCliente(bufferLog, clienteConfig);
        free(bufferLog);
        
        
        if (strcmp(clienteConfig->TemJogo, "COM_JOGO") == 0) {
            if(desistirDeResolver() && strcmp(clienteConfig->tipoJogo,"SIG") == 0){
                pthread_mutex_lock(&mutexSTDOUT);
                printf("Cliente ID:%d\n", clienteConfig->idCliente);
                printf("Desistiu de resolver o jogo\n");
                pthread_mutex_unlock(&mutexSTDOUT);
                logQueEventoCliente(7, *clienteConfig);
            break;
            }
            
            if (!clienteConfig->jogoAtual.resolvido) {
                if (!processarEstadoJogo(clienteConfig)) {
                    break;
                }
            }
            else {
                imprimirResultadoFinal(clienteConfig);
                logQueEventoCliente(8, *clienteConfig);
                // sprintf(resolvidoEmTempo, "Resolvido em %d tentativas e %d ", clienteConfig->jogoAtual.numeroTentativas);
                // logEventoCliente("Cliente resolveu o jogo", clienteConfig);
            }
            
        }
        
        
    }

    if (bytesRead == 0) {
        logQueEventoCliente(6, *clienteConfig);
    } else if (bytesRead < 0) {
        logQueEventoCliente(7, *clienteConfig);
    }
}
// Thread function that will replace the child process logic
void* jogadorThread(void* arg) {
    struct ClienteConfig* config = (struct ClienteConfig*)arg;
    
    pthread_mutex_lock(&mutexSTDOUT);
    printf("Iniciando jogador %d\n", config->idCliente);
    pthread_mutex_unlock(&mutexSTDOUT);

    // Inicia o cliente
    construtorCliente(AF_INET, config->porta, INADDR_ANY, config);
    iniciarClienteSocket(config);

    pthread_exit(NULL);
}
bool desistirDeResolver(){
    //probalidade de 2% de desistir
    srand(pthread_self());
    int desistir = rand() % 100;
    if(desistir < 2){
        return true;
    }
    return false;
}
int main(int argc, char **argv) {
    struct ClienteConfig clienteConfig = {0};

    for(int i = 0; i < 4; i++){
        sem_init(&garfos[i], 0, 1);
    }
    if(sem_init(&mutexFilosofos, 0, 1) != 0){
        perror("Erro ao inicializar semaforo");
        exit(1);
    }
    if(sem_init(&aguardaChegarClientes, 0, 4) != 0){
        perror("Erro ao inicializar semaforo");
        exit(1);
    }
    for(int i = 0; i < 4; i++){
        estadosFilosofos[i] = PENSANDO;
    }
    // Validação dos argumentos da linha de comando
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Validação do nome do arquivo de configuração
    if (!validarNomeFile(argv[1], padrao)) {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }

    // Carrega configurações do arquivo
    carregarConfigCliente(argv[1], &clienteConfig);

    // Número de threads jogadores a criar
    int numJogadores = clienteConfig.numJogadoresASimular;
    
    // Array para guardar os IDs das threads
    pthread_t* threads = calloc(numJogadores, sizeof(pthread_t));
    // Array para configurações específicas de cada thread
    struct ClienteConfig* configsJogadores = calloc(numJogadores, sizeof(struct ClienteConfig));
    
    if (!threads || !configsJogadores) {
        perror("Erro na alocação de memória");
        free(threads);
        free(configsJogadores);
        return 1;
    }
	
    // Cria as threads jogadoras
    for (int i = 0; i < numJogadores; i++) {
        // Copia a configuração base para cada jogador
        configsJogadores[i] = clienteConfig;
        configsJogadores[i].idCliente = i + 1;
        
        int result = pthread_create(&threads[i], NULL, jogadorThread, &configsJogadores[i]);
        //nao é necessário este sleep podendo ser usado para simular diferentes tempos de entrada
        //usar este sleep fica mais claro nas logs que está a fazer certo
        srand(getpid());
        //entre 5ms-10ms
        usleep((rand() % 9501) + 5000);
        
        if (result != 0) {
            fprintf(stderr, "Erro ao criar thread: %s\n", strerror(result));
            // Espera pelas threads já criadas terminarem
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(configsJogadores);
            return 1;
        }

    }
    // Espera todas as threads terminarem
    for (int i = 0; i < numJogadores; i++) {
        pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < numJogadores; i++) {
        printf("Jogador terminou. (%d/%d)\n",  i + 1, numJogadores);
    }

    // Limpa a memória alocada
    free(threads);
    free(configsJogadores);

    printf("Todos os jogadores terminaram.\n");
    return 0;
}