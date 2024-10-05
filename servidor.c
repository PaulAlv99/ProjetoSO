#include "config.h"
#define CONFIGFILE "servidor.conf"

void criarServidor(){

};

ServidorConfig* carregarServidorConfig(char* filename, int* total) {
	int capacidade = CAPACIDADE_CONFIG;  // Capacidade inicial de configurações
    FILE* conf = fopen(filename, "r");

    if (conf == NULL) {
        printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
        return NULL;
    }

    char* line = (char*)malloc(BUF_SIZE * sizeof(char));
    int result_count = 0;
    
    ServidorConfig* results = (ServidorConfig*)malloc(capacidade * sizeof(ServidorConfig));

    while (fgets(line, BUF_SIZE, conf) != NULL) {
        // Se a capacidade for excedida, aloca mais espaço
        if (result_count >= capacidade) {
            capacidade *= 2;
            results = (ServidorConfig*)realloc(results, capacidade * sizeof(ServidorConfig));
        }

        // Leitura do IdJogo (primeira linha)
        results[result_count].idJogo = atoi(line);

        // Leitura do Jogo (segunda linha)
        if (fgets(line, BUF_SIZE, conf) != NULL) {
            strncpy(results[result_count].jogoAtual, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            results[result_count].jogoAtual[strcspn(results[result_count].jogoAtual, "\n")] = '\0';
        }

        // Leitura da Solução (terceira linha)
        if (fgets(line, BUF_SIZE, conf) != NULL) {
            strncpy(results[result_count].solucaoJogo, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            results[result_count].solucaoJogo[strcspn(results[result_count].solucaoJogo, "\n")] = '\0';
        }

        result_count++;  // Contar o número de configurações lidas
    }

    fclose(conf);
    free(line);

    *total = result_count;
    return results;  // Retorna a lista de configurações
}


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

	if (strcmp(argv[1], CONFIGFILE) != 0)
	{
		printf("Nome do ficheiro incorreto");
		return 1;
	}

	int total = 0;
    ServidorConfig* configs = carregarServidorConfig(argv[1], &total);

    if (configs != NULL) {
        for (int i = 0; i < total; i++) {
            printf("Jogo %d: %s\n", configs[i].idJogo, configs[i].jogoAtual);
            printf("Solução: %s\n", configs[i].solucaoJogo);
        }

        free(configs);  // Libera a memória após o uso
    }

    return 0;
}