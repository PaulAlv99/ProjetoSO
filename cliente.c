#include "config.h"
// Aceita cliente1.conf até clienteN.conf
char *padrao = "^cliente[1-9][0-9]*\\.conf$";

#define LINE_SIZE 16

//structs
struct ClienteConfig clienteConfig;

//tricos
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;

void imprimirTabuleiro(char* jogo) {
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        if (i % 3 == 0 && i != 0) {
            printf("---------------------\n");  // Linha separadora horizontal
        }
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (j % 3 == 0 && j != 0) {
                printf(" | ");  // Separador vertical
            }
            printf("%c ", jogo[i * TAMANHO_TABULEIRO + j]);  // Imprime espaço para 0
        }
        printf("\n");
    }
}

// Função para carregar as configurações do cliente
void carregarConfigCliente(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;

    while (fgets(buffer, BUF_SIZE, config) != NULL) {
        // Leitura do IdCliente (primeira linha)
        clienteConfig.idCliente = atoi(buffer);

        // Leitura do IP do servidor (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL) {
            char *resultado = strtok(buffer, "\n");
            strcpy(clienteConfig.ipServidor, resultado);
        }

        contadorConfigs++;  // Contar o número de configurações lidas
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
    return;
}

void logEventoCliente(const char* message) {
    pthread_mutex_lock(&mutexClienteLog);
    //modo append
    FILE *file = fopen("LogCliente.txt", "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexClienteLog);
        return;
    }
    fprintf(file, "[%s] [Client ID: %d] %s\n", getTempo(), clienteConfig.idCliente, message);
    
    fclose(file);
    pthread_mutex_unlock(&mutexClienteLog);
}

void logQueEventoCliente(int numero){
    switch(numero){
        case 1:
            logEventoCliente("Cliente id: iniciou");
            break;
        case 2:
            logEventoCliente("Cliente id: parou");
            break;
        case 3:
            logEventoCliente("Cliente id: conectou-se ao servidor");
            break;
        case 4:
            logEventoCliente("Cliente id: enviou uma mensagem ao servidor");
            break;
        case 5:
            logEventoCliente("Cliente id: recebeu uma resposta do servidor");
            break;
        case 6:
            logEventoCliente("Cliente id: desconectou-se do servidor");
            break;
        default:
            logEventoCliente("Evento desconhecido");
            break;
    }
}

int main(int argc, char **argv) {
    // Verifica se foi fornecido um nome de arquivo
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Valida o nome do arquivo passado como argumento
    if (!validarNomeFile(argv[1], padrao)) {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }

    carregarConfigCliente(argv[1]);
    logEventoCliente("Cliente iniciado");
    return 0;
}