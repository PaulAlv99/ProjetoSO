#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>

//1KB
#define BUF_SIZE 128
#define PATH_SIZE 128 
#define IP_SIZE 16
#define CAPACIDADE_CONFSERVER 1
#define TAMANHO_TABULEIRO 9

// Definição das structs
struct ServidorConfig{
    char ficheiroJogosCaminho[PATH_SIZE];
    char ficheiroSolucoesCaminho[PATH_SIZE];
};

struct ClienteConfig{
    int idCliente;
    char ipServidor[IP_SIZE];
};

//Funcoes

int validarNomeFile(char* arquivoNome,char* padrao){
    regex_t regex;
  
    if (regcomp(&regex, padrao, REG_EXTENDED) != 0) {
        printf("Erro ao compilar a expressão regular.\n");
        return 0;
    }
    // Se o resultado for 0, o nome do arquivo é válido
    int resultado = regexec(&regex, arquivoNome, 0, NULL, 0);
    
    // Liberta mem usada pelo regex
    regfree(&regex);
    
    
    return (resultado == 0);
}

FILE* abrirFicheiro(char* filename){
    FILE* conf = fopen(filename, "r");

    if (conf == NULL) {
        printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
        exit(1);
    }
    return conf;
}

int fecharFicheiro(FILE* file) {
    if (file == NULL) {
        // Não há arquivo para fechar
        printf("Nenhum arquivo foi aberto.\n");
        return 1;
    }

    if (fclose(file) == EOF) {
        // Houve um erro ao tentar fechar o arquivo.
        printf("Erro ao fechar o arquivo.\n");
        return 1;
    }
    return 0; // Indica sucesso.
}

int numeroConfigsFile(FILE* config) {
    int contadorConfigs = 0;
    char line[BUF_SIZE];  // Buffer para armazenar cada linha

    // Conta o número de linhas no ficheiro
    while (fgets(line, sizeof(line), config) != NULL) {
        contadorConfigs++;
    }

    return contadorConfigs;  // Retorna o número de linhas
}

// void* realocarMemoria(void* resultado,size_t tamanhoTipoDados, int capacidade){
//     resultado = realloc(resultado, capacidade * tamanhoTipoDados);
//     if (!resultado) {
//         printf("Erro ao realocar memória.\n");
//         return NULL;
//     }
//     return resultado;
// }

void* alocarMemoria(size_t tamanhoTipoDados, int capacidade) {

    void* resultado = malloc(capacidade * tamanhoTipoDados);
    if (!resultado) {
        printf("Erro ao alocar memória.\n"); 
        return NULL;
    }
    return resultado;
}