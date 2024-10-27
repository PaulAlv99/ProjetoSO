#include "../headers/servidor.h"

pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
void logEventoServidor(const char *message)
{

    // modo append
    char *ficheiroLogs = "servidorLogs/LogServidor.txt";
    pthread_mutex_lock(&mutexServidorLog);
    FILE *file = fopen(ficheiroLogs, "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexServidorLog);
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
    pthread_mutex_unlock(&mutexServidorLog);
}

void logQueEventoServidor(int numero)
{
    switch (numero)
    {
    case 1:
        logEventoServidor("Servidor começou");
        break;
    case 2:
        logEventoServidor("Socket começou");
        break;
    default:
        logEventoServidor("Evento desconhecido");
        break;
    }
}