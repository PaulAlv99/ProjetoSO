#include "config.h"
#define CONFIGFILE "cliente.conf"
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

    //hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
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
    Servidor* configs = carregarServidor(argv[1], &total);

    if (configs != NULL) {
        for (int i = 0; i < total; i++) {
            printf("Jogo %d: %s\n", configs[i].idJogo, configs[i].jogoAtual);
            printf("Solução: %s\n", configs[i].solucaoJogo);
        }

        free(configs);  // Libera a memória após o uso
    }

    return 0;
    //hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
}

Cliente* CarregarClienteConf(int* nomeficheiro, int* totalconf){

    FILE* ficheiroconfig = fopen(nomeficheiro, "r");

    if(ficheiroconfig == NULL)
    {
        printf("Ocorreu um erro na abertura do ficheiro: %s\n", strerror(errno));
        return NULL;
    }
    int quantidade_linhas_config = 0;
    int capacidade = 1;
    char* linhadoconf = (char*)malloc(BUF_SIZE* sizeof(char));
    Cliente* quantidadeconfigarmazenadas = (Cliente*)malloc(capacidade* sizeof(Cliente));

    while (fgets(linhadoconf, BUF_SIZE, ficheiroconfig) != NULL) 
    {
        if(quantidade_linhas_config==0){
            quantidadeconfigarmazenadas[0].idCliente = atoi(linhadoconf);
        }
        if(quantidade_linhas_config==1){
            quantidadeconfigarmazenadas[1].ipServidor = linhadoconf;  
        }
        quantidade_linhas_config++;
    }
    fclose(ficheiroconfig);
    free(linhadoconf);

    *totalconf = quantidade_linhas_config;
    return quantidadeconfigarmazenadas;  // Retorna a lista de configurações

}

