#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>

//1KB
#define BUF_SIZE 128
#define PATH_SIZE 16 
#define IP_SIZE 16
#define CAPACIDADE_CONFSERVER 1
#define TAMANHO_TABULEIRO 9
#define NUMEROS_NO_JOGO 81
#define TEMPO_TAMANHO 22
// Definição das structs
struct ServidorConfig{
    char ficheiroJogosCaminho[PATH_SIZE];
    char ficheiroSolucoesCaminho[PATH_SIZE];
};

struct ClienteConfig{
    int idCliente;
    char ipServidor[IP_SIZE];
};

struct ClienteThread{
    struct ClienteConfig clienteConfig;
    char tabuleiro[NUMEROS_NO_JOGO];
};

//Funcoes

//globais

const char* getTempo() {
    static char buffer[TEMPO_TAMANHO];
    time_t now = time(NULL);
    strftime(buffer, sizeof(buffer) - 1, "%d-%m-%Y %H:%M:%S", localtime(&now));
    return buffer;
}

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
        printf("Erro ao fechar o arquivo: %s\n", strerror(errno));
        return 1;
    }
    return 0; // Indica sucesso.
}

//servidor

void carregarConfigServidor(char* nomeFicheiro);

//cliente
void imprimirTabuleiro(char* jogo);
void carregarConfigCliente(char* nomeFicheiro);