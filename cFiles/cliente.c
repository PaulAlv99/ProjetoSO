#include "../headers/cliente.h"
// Aceita cliente1.conf até clienteN.conf
char *padrao = "^cliente[1-9][0-9]*\\.conf$";

#define LINE_SIZE 16

//tricos

//struct ClienteConfig clienteConfig;

// void imprimirTabuleiro(char* jogo) {
//     for (int i = 0; i < NUM_LINHAS; i++) {
//         if (i % 3 == 0 && i != 0) {
//             printf("---------------------\n");  // Linha separadora horizontal
//         }
//         for (int j = 0; j < NUM_LINHAS; j++) {
//             if (j % 3 == 0 && j != 0) {
//                 printf(" | ");  // Separador vertical
//             }
//             printf("%c ", jogo[i * NUM_LINHAS + j]);  // Imprime espaço para 0
//         }
//         printf("\n");
//     }
// }

// Função para carregar as configurações do cliente
//Descomentar depois de apagar em sercidor.C
// void carregarConfigCliente(char* nomeFicheiro) {
//     FILE* config = abrirFicheiro(nomeFicheiro);

//     fseek(config, 0, SEEK_END);
//     long tamanhoFicheiro = ftell(config);
//     rewind(config);

//     char buffer[tamanhoFicheiro];
//     int contadorConfigs = 0;

//     while (fgets(buffer, BUF_SIZE, config) != NULL) {
//         // Leitura do IdCliente (primeira linha)
//         clienteConfig.idCliente = atoi(buffer);

//         // Leitura do IP do servidor (segunda linha)
//         if (fgets(buffer, BUF_SIZE, config) != NULL) {
//             char *resultado = strtok(buffer, "\n");
//             strcpy(clienteConfig.ipServidor, resultado);
//         }

//         contadorConfigs++;  // Contar o número de configurações lidas
//     }

//     fecharFicheiro(config);
//     if (contadorConfigs == 0) {
//         printf("Sem configs\n");
//         exit(1);
//     }
//     return;
// }

void enviarMensagens(){
    int sockfd, servlen;
	struct sockaddr_un serv_addr;

	/* Cria socket stream */

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		err_dump("client: can't open stream socket");

	/* Primeiro uma limpeza preventiva!
	   Dados para o socket stream: tipo + nome do ficheiro.
		 O ficheiro identifica o servidor */

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	/* Tenta estabelecer uma ligação. Só funciona se o servidor tiver
		 sido lançado primeiro (o servidor tem de criar o ficheiro e associar
		 o socket ao ficheiro) */

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
		err_dump("client: can't connect to server");

	/* Envia as linhas lidas do teclado para o socket */

	str_cli(stdin, sockfd);

	/* Fecha o socket e termina */

	close(sockfd);
	exit(0);
}

int main(int argc, char **argv) {
    // Verifica se foi fornecido um nome de arquivo
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

    // Valida o nome do arquivo passado como argumento
    if (!validarNomeFile(argv[1], padrao)) {
        printf("Nome do ficheiro de configuracao incorreto: %s\n", argv[1]);
        return 1;
    }

    // carregarConfigCliente(argv[1]);
    // imprimirTabuleiro("530070000600195000098000060800060003400803001700020006060000280000419005000080079");
    // logEventoCliente("Cliente iniciado");
    return 0;
}