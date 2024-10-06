#include "config.h"
// Aceita cliente1.conf até cliente99.conf
char *padrao = "^cliente([1-9][0-9]?|99)\\.conf$";

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

ClienteConfig* carregarConfigCliente(char* filename, int* total) {
    FILE* conf = abrirFicheiro(filename);
    int capacidade = CAPACIDADE_CONFIG;  // Capacidade inicial de configurações
    char* line = (char*)malloc(BUF_SIZE * sizeof(char));
    int result_count = 0;
    
    ClienteConfig* results = (ClienteConfig*)malloc(capacidade * sizeof(ClienteConfig));

    while (fgets(line, BUF_SIZE, conf) != NULL) {
        // Se a capacidade for excedida, aloca mais espaço
        if (result_count >= capacidade) {
            capacidade *= 2;
            results = (ClienteConfig*)realloc(results, capacidade * sizeof(ClienteConfig));
        }

        // Leitura do IdJogo (primeira linha)
        results[result_count].idCliente = atoi(line);

        // Leitura do Jogo (segunda linha)
        if (fgets(line, BUF_SIZE, conf) != NULL) {
            strncpy(results[result_count].ipServidor, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            results[result_count].ipServidor[strcspn(results[result_count].ipServidor, "\n")] = '\0';
        }

        result_count++;  // Contar o número de configurações lidas
    }

    fclose(conf);
    free(line);

    *total = result_count;
    return results;  // Retorna a lista de configurações
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

    int total = 0;
    ClienteConfig* configs = carregarConfigCliente(argv[1], &total);

    if (configs == NULL) {
        return 1;
    }
    for (int i = 0; i < total; i++) {
            printf("ID Cliente: %d\n", configs[i].idCliente);
            printf("IP Servidor: %s\n", configs[i].ipServidor);
        }
        free(configs);  // Libera a memória após o uso
    return 0;
    return 0;
}
