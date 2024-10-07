#include "config.h"
#define CONFIGFILE "servidor.conf"

ServidorConfig* carregarConfigServidor(char* nomeFicheiro, int* total) {
    FILE* config = abrirFicheiro(nomeFicheiro);
	int capacidade = CAPACIDADE_CONFIG;  // Capacidade inicial de configurações

    char* line = (char*)malloc(BUF_SIZE * sizeof(char));
    int contadorConfigs = 0;
    
    ServidorConfig* resultado = (ServidorConfig*)malloc(capacidade * sizeof(ServidorConfig));

    while (fgets(line, BUF_SIZE, config) != NULL) {
        // Se a capacidade for excedida, aloca mais espaço
        if (contadorConfigs >= capacidade) {
            capacidade *= 2;
            resultado = (ServidorConfig*)realloc(resultado, capacidade * sizeof(ServidorConfig));
        }

        // Leitura do IdJogo (primeira linha)
        resultado[contadorConfigs].idJogo = atoi(line);

        // Leitura do Jogo (segunda linha)
        if (fgets(line, BUF_SIZE, config) != NULL) {
            strncpy(resultado[contadorConfigs].jogoAtual, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            resultado[contadorConfigs].jogoAtual[strcspn(resultado[contadorConfigs].jogoAtual, "\n")] = '\0';
        }

        // Leitura da Solução (terceira linha)
        if (fgets(line, BUF_SIZE, config) != NULL) {
            strncpy(resultado[contadorConfigs].solucaoJogo, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            resultado[contadorConfigs].solucaoJogo[strcspn(resultado[contadorConfigs].solucaoJogo, "\n")] = '\0';
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

// int validarSolucao(Cliente cliente){
//     //se nao tiver erradas retorna 0, caso contrário retorna o numero de respostas erradas
//     //inicialmente o cliente nao tem resposta erradas. aparece a 0 as suas respostas erradas
//     //quando faz a validação esse numero é atualizado conforme

// }
// int numeroErradasJogo(int idCliente,char* jogo){

// }
int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

	if (strcmp(argv[1], CONFIGFILE) != 0)
	{
		printf("Nome do ficheiro incorreto\n");
		return 1;
	}

	int totalConfigs = 0;
    ServidorConfig* configs = carregarConfigServidor(argv[1], &totalConfigs);

    if (configs == NULL) {
        return 1;
    }
    for (int i = 0; i < totalConfigs; i++) {
            printf("Jogo %d: %s\n", configs[i].idJogo, configs[i].jogoAtual);
            printf("Solução: %s\n", configs[i].solucaoJogo);
    }
    printf("Total configs:%d\n",totalConfigs);
        free(configs);  // Libera a memória após o uso
    return 0;
}