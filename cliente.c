#include "config.h"

// Função para validar o nome do arquivo com regex
int validarNomeFile(const char* arquivoNome){
    regex_t regex;
    // Aceita cliente1.conf até cliente99.conf
    const char *padrao = "^cliente([1-9][0-9]?|99)\\.conf$";
  
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

int main(int argc, char **argv) {
    // Verifica se foi fornecido um nome de arquivo
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Valida o nome do arquivo passado como argumento
    if (!validarNomeFile(argv[1])) {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
