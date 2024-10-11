#include "config.h"
#define CONFIGFILE "servidor.conf"

//structs
struct ServidorConfig serverConfig;

struct ServidorConfig* carregarConfigServidor(char* nomeFicheiro, int* total) {
    FILE* config = abrirFicheiro(nomeFicheiro);
    int numeroLinhas = numeroConfigsFile(config);
    rewind(config);

    struct ServidorConfig* resultado = (struct ServidorConfig*)alocarMemoria(sizeof(struct ServidorConfig), numeroLinhas);

    int contadorConfigs = 0;
    while (fgets(resultado[contadorConfigs].ficheiroJogosCaminho, PATH_SIZE, config) != NULL) {
        resultado[contadorConfigs].ficheiroJogosCaminho[strcspn(resultado[contadorConfigs].ficheiroJogosCaminho, "\n")] = '\0';

        if (fgets(resultado[contadorConfigs].ficheiroSolucoesCaminho, PATH_SIZE, config) != NULL) {
            resultado[contadorConfigs].ficheiroSolucoesCaminho[strcspn(resultado[contadorConfigs].ficheiroSolucoesCaminho, "\n")] = '\0';
        }
        contadorConfigs++;  // Count the number of configurations read
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
    }

    *total = contadorConfigs;
    return resultado;  // Return the list of configurations
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
    int totalConfigs = 0;
    struct ServidorConfig* configs = carregarConfigServidor(argv[1], &totalConfigs);

    if (configs == NULL) {
        return 1;
    }

    for (int i = 0; i < totalConfigs; i++) {
        printf("loc: %s\n", configs[i].ficheiroJogosCaminho);
        printf("loc: %s\n", configs[i].ficheiroSolucoesCaminho);
    }

    printf("Total de configurações: %d\n", totalConfigs);
    free(configs);  // Libera a memória após o uso

    return 0;
}