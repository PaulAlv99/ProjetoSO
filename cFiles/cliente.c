#include "../headers/cliente.h"
// Aceita cliente1.conf até clienteN.conf
char *padrao = "./configs/cliente.conf";

#define LINE_SIZE 16

// global

// tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;

void carregarConfigCliente(char *nomeFicheiro, struct ClienteConfig *clienteConfig)
{
	FILE *config = abrirFicheiro(nomeFicheiro);

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
	printf("---------------------\n");
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
	printf("---------------------\n");
}

void logEventoCliente(const char *message, struct ClienteConfig clienteConfig)
{
	pthread_mutex_lock(&mutexClienteLog);
	// modo append
	char *str = "logs/clienteLog.txt";
	FILE *file = fopen(str, "a");
	if (file == NULL)
	{
		perror("Erro ao abrir o ficheiro de log");
		pthread_mutex_unlock(&mutexClienteLog);
		return;
	}
	fprintf(file, "[%s] [Cliente ID: %lu] %s\n", getTempo(), clienteConfig.idCliente, message);

	fclose(file);
	pthread_mutex_unlock(&mutexClienteLog);
}

void logQueEventoCliente(int numero, struct ClienteConfig clienteConfig)
{
	switch (numero)
	{
	case 1:
		logEventoCliente("Cliente id: iniciou", clienteConfig);
		break;
	case 2:
		logEventoCliente("Cliente id: parou", clienteConfig);
		break;
	case 3:
		logEventoCliente("Cliente id: conectou-se ao servidor", clienteConfig);
		break;
	case 4:
		logEventoCliente("Cliente id: enviou uma mensagem ao servidor", clienteConfig);
		break;
	case 5:
		logEventoCliente("Cliente id: recebeu uma resposta do servidor", clienteConfig);
		break;
	case 6:
		logEventoCliente("Cliente id: desconectou-se do servidor", clienteConfig);
		break;
	default:
		logEventoCliente("Evento desconhecido", clienteConfig);
		break;
	}
}

void construtorCliente(int dominio, int porta, __u_long interface, struct ClienteConfig *clienteConfig)
{
	strcpy(clienteConfig->TemJogo, "SEM_JOGO");
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

	if (connect(clienteConfig->socket, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor)) == -1)
	{
		perror("Erro ao conectar ao servidor");
		close(clienteConfig->socket);
		exit(1);
	}
	char recebeIDCliente[BUF_SIZE] = {0};
	recv(clienteConfig->socket, recebeIDCliente, BUF_SIZE, 0);
	// cliente recebe id do servidor
	clienteConfig->idCliente = atoi(strtok(recebeIDCliente, "|"));
	printf("========= Ligado =========\n");
	printf("===== IP: %s ======\n", clienteConfig->ipServidor);
	printf("===== Porta: %d =======\n", clienteConfig->porta);
	printf("===== Cliente ID: %lu =======\n\n", clienteConfig->idCliente);
	while (1)
	{
		// SEM_JOGO|IDCLIENTE|TIPOJOGO|TIPORESOLUCAO|JOGO
		mandarETratarMSG(clienteConfig);
	}
}

// Atualiza a tentativaAtual
void tentarSolucaoCompleta(char *tentativaAtual, char *valoresCorretos)
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
	char temp[BUF_SIZE];
	if (strcmp(clienteConfig->TemJogo, "SEM_JOGO") == 0)
	{
		sprintf(temp, "SEM_JOGO|%lu|%s|%s|%li|%s|%s|%d|%lu", clienteConfig->idCliente, clienteConfig->tipoJogo, clienteConfig->tipoResolucao, clienteConfig->jogoAtual.idJogo,"0000", "0000", clienteConfig->jogoAtual.resolvido, clienteConfig->jogoAtual.numeroTentativas);
		send(clienteConfig->socket, temp, BUF_SIZE, 0);
		strcpy(clienteConfig->TemJogo, "COM_JOGO");
	}
	char buffer[BUF_SIZE] = {0};
	int bytesReceived = recv(clienteConfig->socket, buffer, BUF_SIZE, 0);
	if (bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		// printf("Recebido do servidor: %s\n", buffer);
		strcpy(clienteConfig->jogoAtual.jogo,buffer);
	}
	// else if (bytesReceived == 0)
	// {
	// 	printf("Conexão fechada pelo servidor.\n");
	// 	close(clienteConfig->socket);
	// 	exit(0);
	// }
	// else
	// {
	// 	perror("Erro ao receber dados do servidor");
	// 	close(clienteConfig->socket);
	// 	exit(1);
	// }
	if (strcmp(clienteConfig->TemJogo, "COM_JOGO") == 0)
	{
		sprintf(temp, "COM_JOGO|%lu|%s|%s|%li|%s|%s|%d|%lu", clienteConfig->idCliente, clienteConfig->tipoJogo, clienteConfig->tipoResolucao, clienteConfig->jogoAtual.idJogo,clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.valoresCorretos, clienteConfig->jogoAtual.resolvido, clienteConfig->jogoAtual.numeroTentativas);
		send(clienteConfig->socket, temp, BUF_SIZE, 0);
		bool resolvido = clienteConfig->jogoAtual.resolvido;
		while (!resolvido)
		{
			if (bytesReceived > 0)
			{ //Recebe correcao do servidor e depois manda outra tentativa
			buffer[bytesReceived] = '\0';
			// printf("Recebido do servidor: %s\n", buffer);
			//char *tempStr = malloc(1024);
    		char *novosValoresCorretos = strtok(buffer, "|");
    		char *logCliente = strtok(NULL, "|");
			char *novasTentativas = strtok(NULL, "|");
			char *novoResolvido = strtok(NULL, "|");
			printf("%s\n", novosValoresCorretos);
			strcpy(clienteConfig->jogoAtual.valoresCorretos, novosValoresCorretos);
			printf("Valores Corretos: %s\n", novosValoresCorretos);
			printf("Valores Corretos: %s\n", clienteConfig->jogoAtual.valoresCorretos);
			logEventoCliente(logCliente, *clienteConfig);
			clienteConfig->jogoAtual.numeroTentativas = (novasTentativas);
			clienteConfig->jogoAtual.resolvido = (bool)(novoResolvido);
			resolvido = (bool)(novoResolvido);
			imprimirTabuleiro(clienteConfig->jogoAtual.valoresCorretos);
			//printf("\n\n");
			//Mandar Nova tentativa
			strcpy(clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.valoresCorretos);
			tentarSolucaoCompleta(clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.valoresCorretos);
			sprintf(temp, "COM_JOGO|%lu|%s|%s|%li|%s|%d", clienteConfig->idCliente, clienteConfig->tipoJogo, clienteConfig->tipoResolucao, clienteConfig->jogoAtual.idJogo, clienteConfig->jogoAtual.jogo, clienteConfig->jogoAtual.resolvido);
			send(clienteConfig->socket, temp, BUF_SIZE, 0);
			imprimirTabuleiro(clienteConfig->jogoAtual.jogo);

			}
			// else if (bytesReceived == 0)
			// {
			// 	printf("Conexão fechada pelo servidor.\n");
			// 	close(clienteConfig->socket);
			// 	exit(0);
			// }
			// else
			// {
			// 	perror("Erro ao receber dados do servidor");
			// 	close(clienteConfig->socket);
			// 	exit(1);
			// }
			
	}

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
	logQueEventoCliente(1, clienteConfig);
	return 0;
}