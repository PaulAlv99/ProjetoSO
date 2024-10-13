#include "config.h"
#define CONFIGFILE "servidor.conf"

//structs
struct ServidorConfig serverConfig;


void carregarConfigServidor(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    //ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
    while (fgets(buffer, PATH_SIZE, config) != NULL) {
        char *resultado = strtok(buffer, "\n");
        strcpy(serverConfig.ficheiroJogosCaminho, resultado);

        if (fgets(buffer, PATH_SIZE, config) != NULL) {
            strcpy(serverConfig.ficheiroSolucoesCaminho, resultado);
        }
        contadorConfigs++;
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
    return;
}

void logEventoServidor(const char* message) {

    //modo append
    FILE *file = fopen("LogServidor.txt", "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
}
void logQueEvento(int numero){
    switch(numero){
        case 1:
            logEventoServidor("Servidor comecou");
            break;
        case 2:
            logEventoServidor("Servidor parou");
            break;
        case 3:
            logEventoServidor("Servidor recebeu novo cliente.");
            break;
        case 4:
            logEventoServidor("Servidor enviou um jogo");
            break;
        case 5:
            logEventoServidor("Servidor recebeu uma solucao.");
            break;
        case 6:
            logEventoServidor("Servidor enviou numero de erradas");
            break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

    if (strcmp(argv[1], CONFIGFILE) != 0) {
        printf("Nome do ficheiro incorreto\n");
        return 1;
    }
    carregarConfigServidor(argv[1]);
    logQueEvento(1);
    return 0;
}