#include "servidor.h"
#define CONFIGFILE "servidor.conf"
#include "cliente.h"
//structs
struct ServidorConfig serverConfig;


void carregarConfigServidor(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    //ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
    while (fgets(buffer, PATH_SIZE, config) != NULL) {
        char *resultado = strtok(buffer, "\n");
        strcpy(serverConfig.ficheiroJogosCaminho, resultado);

        if (fgets(buffer, PATH_SIZE, config) != NULL) {
            strcpy(serverConfig.ficheiroSolucoesCaminho, resultado);
        }
        contadorConfigs++;
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
    return;
}

void logEventoServidor(const char* message) {

    //modo append
    FILE *file = fopen("LogServidor.txt", "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
}
void logQueEventoServidor(int numero){
    switch(numero){
        case 1:
            logEventoServidor("Servidor comecou");
            break;
        case 2:
            logEventoServidor("Servidor parou");
            break;
        case 3:
        //fazer outra funcao para saber ID cliente ex. novo cliente e seu id
            logEventoServidor("O cliente id: conectou-se");
            break;
        case 4:
        //fazer outra funcao para saber ID cliente
            logEventoServidor("Servidor enviou um jogo id cliente:");
            break;
        case 5:
        //fazer outra funcao para saber ID cliente
            logEventoServidor("Servidor recebeu uma solucao.id cliente:");
            break;
        case 6:
        //fazer outra funcao para saber ID cliente
            logEventoServidor("Servidor enviou numero de erradasid cliente:");
            break;
        case 7:
        //fazer outra funcao para saber ID cliente
            logEventoServidor("O cliente id: desconectou-se");
            break;
        default:
            logEventoServidor("Evento desconhecido");
            break;
    }
}
// void verificarLinha(int linha, char* jogo, char* solucao) {
//     //Verifica uma linha do Sudoku (9 posições)
//     for (int i = 0; i < NUM_LINHAS; i++) {
//         int pos = linha * 9 + i;
//         //Verifica se a célula foi preenchida no jogo
//         if (jogo[pos] != solucao[pos]) {
//             //Imprime posição na forma posx-y comecando em 1
//             int coluna = (pos % 9) + 1;
//             printf("Posição errada: pos%d-%d\n", linha + 1, coluna);
//             //começa a contar do 0. pode ser guardado numa estrutura de dados para
//             //posteriormente mandar para o cliente
//             printf("Posicao na string: %d\n",pos);
//         }
//         else{
//             //Imprime posição na forma posx-y comecando em 1
//             int coluna = (pos % 9) + 1;
//             printf("Posição Correta: pos%d-%d\n", linha + 1, coluna);
//             //começa a contar do 0. pode ser guardado numa estrutura de dados para
//             //posteriormente mandar para o cliente
//             printf("Posicao na string: %d\n",pos);
//         }
//     }
// }

// void resolveJogo(char* jogo, char* solucao) {
//     //Verifica linha por linha (0 a 8, correspondente às 9 linhas)
//     for (int linha = 0; linha < 9; linha++) {
//         verificarLinha(linha, jogo, solucao);
//     }
// }

void imprimirTabuleiro(char jogo[]) {
    for (int i = 0; i < NUM_LINHAS; i++) {
        if (i % 3 == 0 && i != 0) {
            printf("---------------------\n");  // Linha separadora horizontal
        }
        for (int j = 0; j < NUM_COLUNAS; j++) {
            if (j % 3 == 0 && j != 0) {
                printf(" | ");  // Separador vertical
            }
            printf("%c ", jogo[i * NUM_LINHAS + j]);
        }
        printf("\n");
    }
}
//Atualiza a tentativaAtual
void tentarSolucaoCompleta(char tentativaAtual[], char valoresCorretos[]){
    for(int i=0; i<strlen(tentativaAtual); i++){
        if((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i])){
            char numero = tentativaAtual[i];
            int numeroInt = (int)(numero);
            int novoNumero = numeroInt + 1;
            char novoNumeroChar = (char)(novoNumero);
            tentativaAtual[i] = novoNumeroChar;
        }
        else if(tentativaAtual[i] == '0'){
            tentativaAtual[i] = '1';
        }
    }
}

//atualiza os valoresCorretos da Ultima Tentativa
void atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas){
    char Tentativas[100];
    sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
    logEventoCliente(Tentativas);
    for(int i = 0; i< strlen(tentativaAtual); i++){
        if(valoresCorretos[i] == '0'){
            if(tentativaAtual[i] == solucao[i]){
                valoresCorretos[i] = tentativaAtual[i];
                char message[1024];
                sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i+1);
                logEventoCliente(message);
                printf(message);

                //printf("%d \n", valoresCorretos);
            }
            else{
                char message[1024];
                sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i+1);
                logEventoCliente(message);
                printf(message);
            }
            
        }
    }
}

//Atualiza o booleano Resolvido se o jogo tiver sido resolvido
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido){
     for(int i = 0; i< strlen(valoresCorretos); i++){
         if(valoresCorretos[i] != solucao[i]){
             return false;
         }
     }
     return true;
}

//ResolveJogo
void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas){
    char tentativaAtual[NUMEROS_NO_JOGO];
    char valoresCorretos[NUMEROS_NO_JOGO];
    strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
    strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
    bool resolvido;
    resolvido = false;
    printf("Jogo Inicial: \n \n");
    imprimirTabuleiro(jogo);
    while(!resolvido){
    //for(int i = 0; i< 10; i++){

        nTentativas = nTentativas + 1;
        tentarSolucaoCompleta(tentativaAtual, valoresCorretos);
        atualizaValoresCorretosCompletos(tentativaAtual, valoresCorretos, solucao, nTentativas);
        resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

        printf("tentativaAtual: \n");
        imprimirTabuleiro(tentativaAtual);

         printf("ValoresCorretos: \n");
        imprimirTabuleiro(valoresCorretos);

        printf("Solução obtida até o momento: \n");
        imprimirTabuleiro(valoresCorretos);
    }
    //}
    printf("Parabéns, esta é a resolução correta! \n");
    char TentativasTotais[100];    
    sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
    logEventoCliente(TentativasTotais);
    printf(TentativasTotais);
}

//Solucao parcial (Tentar um valor por vez)
void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[]){
    for(int i=0; i<strlen(tentativaAtual); i++){
        if((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i])){
            char numero = tentativaAtual[i];
            int numeroInt = (int)(numero);
            int novoNumero = numeroInt + 1;
            char novoNumeroChar = (char)(novoNumero);
            tentativaAtual[i] = novoNumeroChar;
            break;
        }
        else if(tentativaAtual[i] == '0'){
            tentativaAtual[i] = '1';
            break;
        }
    }
}

void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas){
    char Tentativas[100];
    sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
    logEventoCliente(Tentativas);
    for(int i = 0; i< strlen(tentativaAtual); i++){
        if(valoresCorretos[i] == '0'){
            if(tentativaAtual[i] == solucao[i]){
                valoresCorretos[i] = tentativaAtual[i];
                char message[1024];
                sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i+1);
                logEventoCliente(message);
                printf(message);
                return;
                //printf("%d \n", valoresCorretos);
            }
            else{
                char message[1024];
                sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i+1);
                logEventoCliente(message);
                printf(message);
                return;
            }
            
        }
    }
}


//ResolveJogo
void resolverJogoParcial(char jogo[], char solucao[], int nTentativas){
    char tentativaAtual[NUMEROS_NO_JOGO];
    char valoresCorretos[NUMEROS_NO_JOGO];
    strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
    strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
    bool resolvido;
    resolvido = false;
    printf("Jogo Inicial: \n \n");
    imprimirTabuleiro(jogo);
    while(!resolvido){
    //for(int i = 0; i< 10; i++){
        nTentativas = nTentativas + 1;
        tentarSolucaoParcial(tentativaAtual, valoresCorretos);
        atualizaValoresCorretosParcial(tentativaAtual, valoresCorretos, solucao, nTentativas);
        resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

        printf("tentativaAtual: \n");
        imprimirTabuleiro(tentativaAtual);

         printf("ValoresCorretos: \n");
        imprimirTabuleiro(valoresCorretos);

        printf("Solução obtida até o momento: \n");
        imprimirTabuleiro(valoresCorretos);
    }
    //}
    printf("Parabéns, esta é a resolução correta! \n");
    char TentativasTotais[100];    
    sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
    logEventoCliente(TentativasTotais);
    printf(TentativasTotais);
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
    carregarConfigServidor(argv[1]);
    logQueEventoServidor(1);
    char solucao[]="534678912672195348198342567859761423426853791713924856961537284287419635345286179";
    char jogo[]="530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //char valoresCorretos[] = "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //char tentativa[] = "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //printf("jogo 0 = 5? %d ", jogo[0] == '5');
    //printf("%d", valoresCorretos);
    clock_t startTempo, endTempo;
    double tempoResolver;
    startTempo = clock();

    //resolveJogo(jogo,solucao);
    //resolverJogoCompleto(jogo, solucao, 0);
    resolverJogoParcial(jogo, solucao, 0);

    endTempo = clock();
    tempoResolver = (double)(endTempo - startTempo) / CLOCKS_PER_SEC;
    printf("Tempo para resolver: %f seconds\n", tempoResolver);
    //imprimirTabuleiro(jogo);
    return 0;
}