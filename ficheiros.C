#include "ficheiros.h"

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

FILE* abrirFicheiro(char* filename){
    FILE* conf = fopen(filename, "r");

    if (conf == NULL) {
        printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
        exit(1);
    }
    return conf;
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