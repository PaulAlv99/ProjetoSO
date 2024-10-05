#include "config.h"

// Função para validar o nome do arquivo com regex
int validarNomeFile(char* arquivoNome){

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

Cliente* CarregarClienteConf(int* nomeficheiro, int* totalconf){

    FILE* ficheiroconfig = fopen(nomeficheiro, "r");

    if(ficheiroconfig == NULL)
    {
        printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
        return NULL;
    }
    int quantidade_config = 0;
    int capacidade = 1;
    char* linhadoconf = (char*)malloc(BUF_SIZE* sizeof(char));
    Cliente* quantidadeconfigarmazenadas = (Cliente*)malloc(capacidade* sizeof(Cliente));

    while (fgets(linhadoconf, BUF_SIZE, ficheiroconfig) != NULL) 
    {
        if(quantidade_config==0){
            quantidadeconfigarmazenadas[0].idCliente = atoi(linhadoconf);
        }
        if(quantidade_config==1){
            quantidadeconfigarmazenadas[1].ipServidor = linhadoconf;  
        }
    }
    quantidadeconfigarmazenadas[0].idCliente = atoi(linhadoconf);

    if()
    quantidadeconfigarmazenadas[1].ipServidor = 

    while (fgets(linhadoconf, BUF_SIZE, config) != NULL) {
        // Leitura do IdJogo (primeira linha)
        quantidadeconfigarmazenadas[quantidade_config].idCliente = atoi(linhadoconf);

        // Leitura do Jogo (segunda linha)
        if (fgets(linhadoconf, BUF_SIZE, config) != NULL) {

            strncpy(quantidadeconfigarmazenadas[quantidade_config].ipServidor, linhadoconf, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            quantidadeconfigarmazenadas[quantidade_config].jogoAtual[strcspn(quantidadeconfigarmazenadas[quantidade_config].jogoAtual, "\n")] = '\0';
        }

        // Leitura da Solução (terceira linha)
        if (fgets(line, BUF_SIZE, config) != NULL) {
            strncpy(quantidadeconfigarmazenadas[quantidade_config].solucaoJogo, line, BUF_SIZE);
            // Remove o newline '\n' ao final da string, se existir
            quantidadeconfigarmazenadas[quantidade_config].solucaoJogo[strcspn(quantidadeconfigarmazenadas[quantidade_config].solucaoJogo, "\n")] = '\0';
        }

        quantidade_config++;  // Contar o número de configurações lidas
    }

}