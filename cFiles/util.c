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
    ssize_t totalRead = 0;
    char *ptr = (char *)buffer;
    
    // Set socket timeout
    struct timespec tv;
    tv.tv_sec = 5;  // 5 second timeout
    tv.tv_nsec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // Use poll to check for data availability
    struct pollfd pfd = {
        .fd = socket,
        .events = POLLIN,
    };

    while (length > 0)
    {
        // Check if data is available
        int poll_result = poll(&pfd, 1, 1000); // 1 second timeout
        if (poll_result < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (poll_result == 0)
        {
            // Timeout - return what we have or -1 if nothing read
            return totalRead > 0 ? totalRead : -1;
        }

        ssize_t n = recv(socket, ptr, length, 0);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return totalRead > 0 ? totalRead : -1;
            if (errno == ECONNRESET || errno == EPIPE)
            {
                printf("[Sistema] Conexão encerrada pelo cliente\n");
                return -1;
            }
            return -1;
        }
        if (n == 0)
            return totalRead; // Connection closed

        totalRead += n;
        ptr += n;
        length -= n;

        // If we have some data and no more is immediately available, return
        if (totalRead > 0 && poll(&pfd, 1, 0) <= 0)
            break;
    }
    return totalRead;
}

ssize_t writeSocket(int socket, const void *buffer, size_t length)
{
    ssize_t written = 0;
    const char *ptr = (const char *)buffer;
    
    // Set socket timeout
    struct timespec tv;
    tv.tv_sec = 5;
    tv.tv_nsec = 0;
    setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    // Use poll to check for write availability
    struct pollfd pfd = {
        .fd = socket,
        .events = POLLOUT,
    };

    while (length > 0)
    {
        // Check if we can write
        int poll_result = poll(&pfd, 1, 1000); // 1 second timeout
        if (poll_result < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (poll_result == 0)
        {
            // Timeout - return what we've written or -1 if nothing written
            return written > 0 ? written : -1;
        }

        ssize_t n = send(socket, ptr, length, MSG_NOSIGNAL);
        if (n <= 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return written > 0 ? written : -1;
            if (errno == EPIPE)
            {
                printf("[Sistema] Cliente desconectou abruptamente\n");
                return -1;
            }
            return -1;
        }

        written += n;
        ptr += n;
        length -= n;
    }
    return written;
}