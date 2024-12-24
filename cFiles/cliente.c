#include "../headers/cliente.h"
// Aceita cliente1.conf até clienteN.conf
// TODO locks para escrever no stdout
char *padrao = "./configs/cliente";

#define LINE_SIZE 16

// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;
sem_t semSTDOUT;


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
            case 4:
				clienteConfig->numJogadoresASimular = atoi(valor);
				break;
			case 5:
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
	fprintf(file, "[%s] [Cliente ID: %u] %s\n", getTempo(), clienteConfig->idCliente, message);

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
	case 2:
		logEventoCliente("Cliente desconectou-se", &clienteConfig);
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
	default:
		logEventoCliente("Evento desconhecido", &clienteConfig);
		break;
	}
}

void construtorCliente(int dominio, int porta, __u_long interface, struct ClienteConfig *clienteConfig)
{
	strcpy(clienteConfig->TemJogo, "SEM_JOGO");
	clienteConfig->jogoAtual.resolvido = false;
	clienteConfig->jogoAtual.numeroTentativas = 1;
	clienteConfig->jogoAtual.idJogo = 0;
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
	while (connect(clienteConfig->socket, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) == -1)
	{
		if (umaVez)
		{
			perror("Erro ao conectar ao servidor");
			printf("Tentando conectar ao servidor...\n");
			umaVez = 0;
		}
	}
	char recebeIDCliente[BUF_SIZE] = {0};
	readSocket(clienteConfig->socket, recebeIDCliente, BUF_SIZE);
	// cliente recebe id do servidor
	clienteConfig->idCliente = atoi(strtok(recebeIDCliente, "|"));
	logQueEventoCliente(1, *clienteConfig);
	
	printf("========= Ligado =========\n");
	printf("===== IP: %s ======\n", clienteConfig->ipServidor);
	printf("===== Porta: %d =======\n", clienteConfig->porta);
	printf("===== Cliente ID: %u =======\n\n", clienteConfig->idCliente);
	sem_post(&semSTDOUT);
	mandarETratarMSG(clienteConfig);
	close(clienteConfig->socket);
	logQueEventoCliente(2, *clienteConfig);
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

void mandarETratarMSG(struct ClienteConfig *clienteConfig)
{

	char buffer[BUF_SIZE] = {0};

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

	writeSocket(clienteConfig->socket, buffer, BUF_SIZE);

	memset(buffer, 0, BUF_SIZE);

	readSocket(clienteConfig->socket, buffer, BUF_SIZE);
	char* filaCheia="FILA CHEIA SINGLEPLAYER";
	if(strcmp(buffer,filaCheia)==0){
		sem_wait(&semSTDOUT);
		printf("Fila singleplayer está cheia\n");
		sem_post(&semSTDOUT);
		return;
	}
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
	if (idCliente == NULL || tipoJogo == NULL || tipoResolucao == NULL || temJogo == NULL || idJogo == NULL || jogo == NULL || valoresCorretos == NULL || tempoInicio == NULL || tempoFinal == NULL || resolvido == NULL || numeroTentativas == NULL)
	{
		printf("Erro: Falha ao ler\n");
		exit(1);
	}
	clienteConfig->idCliente = atoi(idCliente);
	strcpy(clienteConfig->TemJogo, temJogo);
	clienteConfig->jogoAtual.idJogo = atoi(idJogo);
	strcpy(clienteConfig->jogoAtual.jogo, jogo);
	strcpy(clienteConfig->jogoAtual.valoresCorretos, valoresCorretos);
	strcpy(clienteConfig->jogoAtual.tempoInicio, tempoInicio);
	strcpy(clienteConfig->jogoAtual.tempoFinal, tempoFinal);
	clienteConfig->jogoAtual.resolvido = atoi(resolvido);
	clienteConfig->jogoAtual.numeroTentativas = atoi(numeroTentativas);
	if (strcmp(clienteConfig->TemJogo, "COM_JOGO") == 0)
	{
		sem_wait(&semSTDOUT);
		printf("Cliente ID:%d\n", clienteConfig->idCliente);
		printf("Iniciando tentativa de solução...\n");
		printf("Jogo Inicial:\n\n");
		printf("Hora de inicio: %s\n\n", clienteConfig->jogoAtual.tempoInicio);
		imprimirTabuleiro(clienteConfig->jogoAtual.jogo);
		sem_post(&semSTDOUT);
		while (!clienteConfig->jogoAtual.resolvido)
		{
			char bufferEnviar[BUF_SIZE] = {0};
			if (strcmp(clienteConfig->tipoResolucao, "COMPLET") == 0)
			{
				tentarSolucaoCompleta(clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.valoresCorretos);
			}
			else if (strcmp(clienteConfig->tipoResolucao, "PARCIAL") == 0)
			{
				tentarSolucaoParcial(clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.valoresCorretos);
				// clienteConfig->jogoAtual.numeroTentativas++;
			}
			sem_wait(&semSTDOUT);
			printf("Cliente ID:%d\n", clienteConfig->idCliente);
			printf("\nTentativa %d:\n\n", clienteConfig->jogoAtual.numeroTentativas);
			imprimirTabuleiro(clienteConfig->jogoAtual.jogo);
			sem_post(&semSTDOUT);

			// Enviar tentativa
			sprintf(bufferEnviar, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d",
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

			//tempo entre tentativas
			printf("Tempo entre tentativas: %d\n", clienteConfig->tempoEntreTentativas);
			struct timespec tempo;
			//convert ms da config para timespec- em sec e ns
			//para retirar do numero em ms os segundos primeiro e os ns
			tempo.tv_sec = clienteConfig->tempoEntreTentativas / 1000;
			tempo.tv_nsec = (clienteConfig->tempoEntreTentativas % 1000) * 1000000;
			nanosleep(&tempo, NULL);
			writeSocket(clienteConfig->socket, bufferEnviar, BUF_SIZE);
			char *bufferEnviarFinal = malloc(2*BUF_SIZE * sizeof(char));
			if (bufferEnviarFinal == NULL)
			{
				perror("Erro ao alocar memoria");
				exit(1);
			}
			sprintf(bufferEnviarFinal, "Mensagem enviada: %s", bufferEnviar);
			logEventoCliente(bufferEnviarFinal, clienteConfig);
			free(bufferEnviarFinal);
			// sem_post(&semAguardar);
			// Limpar buffer antes de receber
			memset(buffer, 0, BUF_SIZE);
			// Receber resposta
			if (recv(clienteConfig->socket, buffer, BUF_SIZE, 0) > 0)
			{
				char *bufferFinal = malloc(2*BUF_SIZE * sizeof(char));
				if (bufferFinal == NULL)
				{
					perror("Erro ao alocar memoria");
					exit(1);
				}
				sprintf(bufferFinal, "Mensagem recebida: %s", buffer);
				logEventoCliente(bufferFinal, clienteConfig);
				free(bufferFinal);
				// Parse da mensagem recebida (uma única vez)
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
				char *logCliente = strtok(NULL, "|");

				// Verificar se todos os campos foram lidos corretamente
				if (idCliente && tipoJogo && tipoResolucao && temJogo &&
					idJogo && jogo && valoresCorretos && tempoInicio &&
					tempoFinal && resolvido && numeroTentativas && logCliente)
				{
					clienteConfig->idCliente = atoi(idCliente);
					strcpy(clienteConfig->TemJogo, temJogo);
					clienteConfig->jogoAtual.idJogo = atoi(idJogo);
					strcpy(clienteConfig->jogoAtual.jogo, jogo);
					strcpy(clienteConfig->jogoAtual.valoresCorretos, valoresCorretos);
					strcpy(clienteConfig->jogoAtual.tempoInicio, tempoInicio);
					strcpy(clienteConfig->jogoAtual.tempoFinal, tempoFinal);
					clienteConfig->jogoAtual.resolvido = atoi(resolvido);
					clienteConfig->jogoAtual.numeroTentativas = atoi(numeroTentativas);
					logEventoCliente(logCliente, clienteConfig);
					sem_wait(&semSTDOUT);
					printf("Cliente ID:%d\n", clienteConfig->idCliente);
					printf("Valores corretos:\n\n");
					imprimirTabuleiro(clienteConfig->jogoAtual.valoresCorretos);
					sem_post(&semSTDOUT);
				}
				else
				{
					printf("Erro: Mensagem recebida com formato inválido\n");
				}
			}
			else
			{
				perror("Erro ao receber mensagem do servidor");
				exit(1);
			}
		}
		sem_wait(&semSTDOUT);
		printf("Jogo resolvido!\n");
		printf("Resolvido em %d tentativas\n", clienteConfig->jogoAtual.numeroTentativas - 1);
		printf("Hora de fim: %s\n", clienteConfig->jogoAtual.tempoFinal);
		sem_post(&semSTDOUT);
	}
}
// Thread function that will replace the child process logic
void* jogadorThread(void* arg) {
    struct ClienteConfig* config = (struct ClienteConfig*)arg;
    
    sem_wait(&semSTDOUT);
    printf("Iniciando jogador %d\n", config->idCliente);
    sem_post(&semSTDOUT);

    // Inicia o cliente
    construtorCliente(AF_INET, config->porta, INADDR_ANY, config);
    iniciarClienteSocket(config);

    sem_wait(&semSTDOUT);
    printf("Finalizando jogador %d\n", config->idCliente);
    sem_post(&semSTDOUT);

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    struct ClienteConfig clienteConfig = {0};
	sem_init(&semSTDOUT, 0, 1);
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
    pthread_t* threads = malloc(numJogadores * sizeof(pthread_t));
    // Array para configurações específicas de cada thread
    struct ClienteConfig* configsJogadores = malloc(numJogadores * sizeof(struct ClienteConfig));
    
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
        usleep(10000); // Mantém o pequeno delay entre criação de jogadores
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < numJogadores; i++) {
        pthread_join(threads[i], NULL);
        printf("Jogador %d terminou. (%d/%d)\n", 
               i + 1, i + 1, numJogadores);
    }

    // Limpa a memória alocada
    free(threads);
    free(configsJogadores);

    printf("Todos os jogadores terminaram.\n");
    return 0;
}