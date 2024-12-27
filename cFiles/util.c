#include "../headers/util.h"

int fecharFicheiro(FILE *file)
{
	if (file == NULL)
	{
		// Não há arquivo para fechar
		printf("Nenhum arquivo foi aberto.\n");
		return 1;
	}

	if (fclose(file) == EOF)
	{
		// Houve um erro ao tentar fechar o arquivo.
		printf("Erro ao fechar o arquivo: %s\n", strerror(errno));
		return 1;
	}
	return 0; // Indica sucesso.
}

FILE *abrirFicheiroRead(char *filename)
{
	FILE *conf = fopen(filename, "r");

	if (conf == NULL)
	{
		printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
		exit(1);
	}
	return conf;
}

int validarNomeFile(char *arquivoNome, char *padrao)
{
	regex_t regex;

	if (regcomp(&regex, padrao, REG_EXTENDED) != 0)
	{
		printf("Erro ao compilar a expressão regular.\n");
		return 0;
	}
	// Se o resultado for 0, o nome do arquivo é válido
	int resultado = regexec(&regex, arquivoNome, 0, NULL, 0);

	// Liberta mem usada pelo regex
	regfree(&regex);

	return (resultado == 0);
}

const char *getTempo()
{
	static char buffer[TEMPO_TAMANHO];
	time_t now = time(NULL);
	strftime(buffer, sizeof(buffer) - 1, "%d-%m-%Y %H:%M:%S", localtime(&now));
	return buffer;
}

const char *getTempoHoraMinutoSegundo()
{
	static char buffer[TEMPO_TAMANHO];
	time_t now = time(NULL);
	strftime(buffer, sizeof(buffer) - 1, "%H:%M:%S", localtime(&now));
	return buffer;
}

// Funcao converter tempo em String para temp em time_t
const time_t converterTempoStringParaTimeT(char *tempo)
{
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	time_t t = 0;
	strptime(tempo, "%H:%M:%S", &tm);
	t = mktime(&tm);
	return t;
}

ssize_t readSocket(int socket, void *buffer, size_t length)
{
    // Single blocking read operation
    ssize_t n = recv(socket, buffer, length, 0);
    if (n < 0)
    {
        if (errno == EINTR)
        {
            return -1; // Interrupted by signal
        }
        if (errno == ECONNRESET || errno == EPIPE)
        {
            printf("[Sistema] Conexão encerrada pelo cliente\n");
            return -1;
        }
        return -1; // Error in reading
    }
    return n;
}

// Socket write function that ensures complete message is sent
ssize_t writeSocket(int socket, const void *buffer, size_t length)
{
    ssize_t written = 0;
    const char *ptr = (const char *)buffer;
    while (length > 0)
    {
        ssize_t n = send(socket, ptr, length, MSG_NOSIGNAL);
        if (n <= 0)
        {
            if (errno == EINTR)
                continue; // Interrupted by signal
            if (errno == EPIPE)
            {
                printf("[Sistema] Cliente desconectou abruptamente\n");
                return -1; // Connection closed
            }
            return -1; // Write error
        }
        written += n;
        ptr += n;
        length -= n;
    }
    return written;
}