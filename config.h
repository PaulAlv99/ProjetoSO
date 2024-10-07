#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>

//1KB
#define BUF_SIZE 128
#define IP_SIZE 16  
#define CAPACIDADE_CONFIG 10
#define TAMANHO_TABULEIRO 9

// Função para validar o nome do arquivo com regex
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
        fclose(conf);
        return NULL;
    }
    return conf;
}

int fecharFicheiro(FILE* file,const char* filename) {
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
    printf("Fechou ficheiro: %s\n",filename);
    return 0; // Indica sucesso.
}

// Definição da struct para armazenar as configurações
typedef struct {
    int idJogo;
    char jogoAtual[BUF_SIZE];
    char solucaoJogo[BUF_SIZE];
} ServidorConfig;

typedef struct {
    int idJogo;
    char jogoAtual[BUF_SIZE];
} Tabuleiro;

typedef struct{
    int idCliente;
    char ipServidor[IP_SIZE];
}ClienteConfig;