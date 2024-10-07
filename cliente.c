#include "config.h"
// Aceita cliente1.conf até cliente99.conf
char *padrao = "^cliente([1-9][0-9]?|99)\\.conf$";
#define LINE_SIZE 16
// void enviarJogoAtual(int idCliente){

// }

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

ClienteConfig* carregarConfigCliente(char* nomeFicheiro, int* total) {
    FILE* config = abrirFicheiro(nomeFicheiro);
    int capacidade = CAPACIDADE_CONFIG;  // Capacidade inicial de configurações
    char* line = (char*)malloc(BUF_SIZE * sizeof(char));
    int contadorConfigs = 0;
    
    ClienteConfig* resultado = (ClienteConfig*)malloc(capacidade * sizeof(ClienteConfig));

    while (fgets(line, BUF_SIZE, config) != NULL) {
        // Se a capacidade for excedida, aloca mais espaço
        if (contadorConfigs >= capacidade) {
            capacidade *= 2;
            resultado = (ClienteConfig*)realloc(resultado, capacidade * sizeof(ClienteConfig));
        }

        // Leitura do IdJogo (primeira linha)
        resultado[contadorConfigs].idCliente = atoi(line);

        // Leitura do Jogo (segunda linha)
        if (fgets(line, BUF_SIZE, config) != NULL) {
            strncpy(resultado[contadorConfigs].ipServidor, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            resultado[contadorConfigs].ipServidor[strcspn(resultado[contadorConfigs].ipServidor, "\n")] = '\0';
        }

        contadorConfigs++;  // Contar o número de configurações lidas
    }

    fecharFicheiro(config,nomeFicheiro);
    if(contadorConfigs==0){
        printf("Sem configs\n");
    }
    free(line);

    *total = contadorConfigs;
    return resultado;  // Retorna a lista de configurações
}


int main(int argc, char **argv) {
    // Verifica se foi fornecido um nome de arquivo
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Valida o nome do arquivo passado como argumento
    if (!validarNomeFile(argv[1],padrao)) {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }

    int totalConfigs = 0;
    ClienteConfig* configs = carregarConfigCliente(argv[1], &totalConfigs);

    if (configs == NULL) {
        return 1;
    }
    for (int i = 0; i < totalConfigs; i++) {
            printf("ID Cliente: %d\n", configs[i].idCliente);
            printf("IP Servidor: %s\n", configs[i].ipServidor);
        }
    printf("Total configs:%d\n",totalConfigs);
        free(configs);  // Libera a memória após o uso
    return 0;
}
