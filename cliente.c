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

ClienteConfig* carregarConfigCliente(char* filename, int* total) {
    FILE* conf = abrirFicheiro(filename);
    
    if (conf == NULL) {
        return NULL; // Handle file opening error
    }

    int capacidade = CAPACIDADE_CONFIG;  // Initial capacity for configurations
    char* line = (char*)malloc(LINE_SIZE * sizeof(char));
    int result_count = 0;
    
    ClienteConfig* results = (ClienteConfig*)malloc(capacidade * sizeof(ClienteConfig));

    while (fgets(line, LINE_SIZE, conf) != NULL) {
        // Check if we need to resize the results array
        if (result_count >= capacidade) {
            int new_capacity = capacidade + 5; // Add a fixed number of slots
            ClienteConfig* temp = (ClienteConfig*)realloc(results, new_capacity * sizeof(ClienteConfig));
            if (temp == NULL) {
                free(results); // Free old results if realloc fails
                free(line);
                fclose(conf);
                return NULL; // Handle realloc failure
            }
            results = temp;
            capacidade = new_capacity; // Update capacity to the new size
        }

        // Read IdCliente (first line)
        results[result_count].idCliente = atoi(line);

        // Read the IP address (second line)
        if (fgets(line, LINE_SIZE, conf) != NULL) {
            strncpy(results[result_count].ipServidor, line, IP_SIZE);
            // Remove newline '\n' from the end of the string, if it exists
            results[result_count].ipServidor[strcspn(results[result_count].ipServidor, "\n")] = '\0';
        }

        result_count++;  // Count the number of configurations read
    }

    fclose(conf);
    free(line);

    *total = result_count;
    return results;  // Return the list of configurations
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
}
