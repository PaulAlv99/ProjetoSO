#include "../headers/cliente.h"
// Aceita cliente1.conf até clienteN.conf
char *padrao = "./configs/cliente";

#define LINE_SIZE 16

// global

// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;

// semaforo
sem_t semAguardar;

void carregarConfigCliente(char *nomeFicheiro, struct ClienteConfig *clienteConfig)
{
	FILE *config = abrirFicheiroRead(nomeFicheiro);

	if (config == NULL)
	{
		fprintf(stderr, "Erro: Falha ao abrir o ficheiro de configuração.\n");
		exit(1);
	}

	char buffer[BUF_SIZE];

	if (fgets(buffer, BUF_SIZE, config) != NULL)
	{
		strncpy(clienteConfig->tipoJogo, strtok(buffer, "\n"), INFO_SIZE);
	}
	else
	{
		fprintf(stderr, "Erro: Falha ao ler tipoJogo.\n");
		fecharFicheiro(config);
		exit(1);
	}
	if (fgets(buffer, BUF_SIZE, config) != NULL)
	{
		strncpy(clienteConfig->tipoResolucao, strtok(buffer, "\n"), INFO_SIZE);
	}
	else
	{
		fprintf(stderr, "Erro: Falha ao ler tipoResolucao.\n");
		fecharFicheiro(config);
		exit(1);
	}
	if (fgets(buffer, BUF_SIZE, config) != NULL)
	{
		// 255.255.255.255(15)
		strncpy(clienteConfig->ipServidor, strtok(buffer, "\n"), IP_SIZE - 1);
	}
	else
	{
		fprintf(stderr, "Erro: Falha ao ler ipServidor.\n");
		fecharFicheiro(config);
		exit(1);
	}
	if (fgets(buffer, BUF_SIZE, config) != NULL)
	{
		clienteConfig->porta = atoi(strtok(buffer, "\n"));
	}
	else
	{
		fprintf(stderr, "Erro: Falha ao ler ipServidor.\n");
		fecharFicheiro(config);
		exit(1);
	}

	fecharFicheiro(config);
	return;
}
// tricos

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
	clienteConfig->jogoAtual.numeroTentativas = 0;
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
	read(clienteConfig->socket, recebeIDCliente, BUF_SIZE);
	// cliente recebe id do servidor
	clienteConfig->idCliente = atoi(strtok(recebeIDCliente, "|"));
	logQueEventoCliente(1, *clienteConfig);
	printf("========= Ligado =========\n");
	printf("===== IP: %s ======\n", clienteConfig->ipServidor);
	printf("===== Porta: %d =======\n", clienteConfig->porta);
	printf("===== Cliente ID: %u =======\n\n", clienteConfig->idCliente);
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
	sem_init(&semAguardar, 0, 1);
	// abrir sem existente
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
	// sem_wait(&semAguardar);
	write(clienteConfig->socket, buffer, BUF_SIZE);
	// sem_post(&semAguardar);
	memset(buffer, 0, BUF_SIZE);
	// sem_wait(&semAguardar);
	read(clienteConfig->socket, buffer, BUF_SIZE);
	// sem_post(&semAguardar);
	// printf("Mensagem recebida: %s\n", buffer);
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
		printf("Iniciando tentativa de solução...\n");
		printf("Jogo Inicial:\n\n");
		printf("Hora de inicio: %s\n\n", clienteConfig->jogoAtual.tempoInicio);
		imprimirTabuleiro(clienteConfig->jogoAtual.jogo);

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
			printf("\nTentativa %d:\n\n", clienteConfig->jogoAtual.numeroTentativas);
			imprimirTabuleiro(clienteConfig->jogoAtual.jogo);

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

			// sem_wait(&semAguardar);
			write(clienteConfig->socket, bufferEnviar, BUF_SIZE);
			char bufferEnviarFinal[BUF_SIZE] = {0};

			sprintf(bufferEnviarFinal, "\nMensagem enviada: %s\n", bufferEnviar);
			logEventoCliente(bufferEnviarFinal, clienteConfig);
			// sem_post(&semAguardar);
			// Limpar buffer antes de receber
			memset(buffer, 0, BUF_SIZE);
			// Receber resposta
			if (recv(clienteConfig->socket, buffer, BUF_SIZE, 0) > 0)
			{
				char bufferFinal[BUF_SIZE] = {0};
				sprintf(bufferFinal, "Mensagem recebida: %s\n", buffer);
				logEventoCliente(bufferFinal, clienteConfig);

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
					printf("Valores corretos:\n\n");
					imprimirTabuleiro(clienteConfig->jogoAtual.valoresCorretos);
				}
				else
				{
					printf("Erro: Mensagem recebida com formato inválido\n");
				}
				// sem_post(&semAguardar);
			}
		}
		strcpy(clienteConfig->jogoAtual.tempoFinal, getTempoHoraMinutoSegundo());
		printf("Jogo resolvido!\n");
		printf("Resolvido em %d tentativas\n", clienteConfig->jogoAtual.numeroTentativas);
	}
}
int main(int argc, char **argv)
{
	struct ClienteConfig clienteConfig = {0};
	// Verifica se foi fornecido um nome de arquivo
	if (argc < 2)
	{
		printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
		return 1;
	}

	// Valida o nome do arquivo passado como argumento
	if (!validarNomeFile(argv[1], padrao))
	{
		printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
		return 1;
	}
	carregarConfigCliente(argv[1], &clienteConfig);
	construtorCliente(AF_INET, clienteConfig.porta, INADDR_ANY, &clienteConfig);
	iniciarClienteSocket(&clienteConfig);
	return 0;
}