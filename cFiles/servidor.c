#include "../headers/servidor.h"


//structs
struct ServidorConfig serverConfig;
struct Jogo jogosEsolucoes[NUM_JOGOS];
//tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteLog = PTHREAD_MUTEX_INITIALIZER;

struct ClienteConfig clienteConfig;


//Isto nao e daqui, apagar depois
void carregarConfigCliente(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;

    while (fgets(buffer, BUF_SIZE, config) != NULL) {
        // Leitura do IdCliente (primeira linha)
        clienteConfig.idCliente = atoi(buffer);

        // Leitura do IP do servidor (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL) {
            char *resultado = strtok(buffer, "\n");
            strcpy(clienteConfig.ipServidor, resultado);
        }

        contadorConfigs++;  // Contar o número de configurações lidas
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
    return;
}

//so tem o ficheiro da localizacao dos jogos e solucoes que neste caso e apenas 1 ficheiro
void carregarConfigServidor(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    //ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
        if (fgets(buffer, PATH_SIZE, config) != NULL) {
            strcpy(serverConfig.ficheiroJogosESolucoesCaminho, buffer);
            // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
        }
        contadorConfigs++;
        fecharFicheiro(config);
        if (contadorConfigs == 0) {
            printf("Sem configs\n");
            exit(1);
        }

    return;
}

void logEventoServidor(const char* message) {

    //modo append
    char* ficheiroLogs="logs/LogServidor.txt";
    pthread_mutex_lock(&mutexServidorLog);
    FILE *file = fopen(ficheiroLogs, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexServidorLog);
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
    pthread_mutex_unlock(&mutexServidorLog);
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

void carregarFicheiroJogosSolucoes(char* nomeFicheiro) {
    FILE* config = abrirFicheiro(nomeFicheiro);
    
    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    while (fgets(buffer, BUF_SIZE, config) != NULL) {
        // Leitura do idJogo (primeira linha)
        char *resultado = strtok(buffer, "\n");
        jogosEsolucoes[contadorConfigs].idJogo = atoi(resultado);
        printf("ID:%ld\n",jogosEsolucoes[contadorConfigs].idJogo);
        // Leitura do IP do servidor (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL) {
            char *resultado = strtok(buffer, "\n");
            strcpy(jogosEsolucoes[contadorConfigs].jogo, resultado);
            printf("Jogo:%s\n",jogosEsolucoes[contadorConfigs].jogo);
            printf("Jogo:%s\n",resultado);
            if (fgets(buffer, BUF_SIZE, config) != NULL) {
                char *resultado = strtok(buffer, "\n");
                strcpy(jogosEsolucoes[contadorConfigs].solucao, resultado);
                printf("Jogo:%s\n",jogosEsolucoes[contadorConfigs].solucao);
                printf("Jogo:%s\n",resultado);
            }
        }

        contadorConfigs++;  // Contar o número de configurações lidas
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0) {
        printf("Sem configs\n");
        exit(1);
    }
    // printf("Configs n:%d\n",contadorConfigs);
    return;
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

void logEventoCliente(const char* message) {
    pthread_mutex_lock(&mutexClienteLog);
    //modo append
    char* str = "logs/clienteLog.txt";
    FILE *file = fopen(str, "a");
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexClienteLog);
        return;
    }
    fprintf(file, "[%s] [Cliente ID: %lu] %s\n", getTempo(), clienteConfig.idCliente, message);
    
    fclose(file);
    pthread_mutex_unlock(&mutexClienteLog);
}

void logQueEventoCliente(int numero){
    switch(numero){
        case 1:
            logEventoCliente("Cliente id: iniciou");
            break;
        case 2:
            logEventoCliente("Cliente id: parou");
            break;
        case 3:
            logEventoCliente("Cliente id: conectou-se ao servidor");
            break;
        case 4:
            logEventoCliente("Cliente id: enviou uma mensagem ao servidor");
            break;
        case 5:
            logEventoCliente("Cliente id: recebeu uma resposta do servidor");
            break;
        case 6:
            logEventoCliente("Cliente id: desconectou-se do servidor");
            break;
        default:
            logEventoCliente("Evento desconhecido");
            break;
    }
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
    printf("Caminho jogos e solucoes: %s",serverConfig.ficheiroJogosESolucoesCaminho);
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho);
    logQueEventoServidor(1);
    // char solucao[]="534678912672195348198342567859761423426853791713924856961537284287419635345286179";
    // char jogo[]="530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //char valoresCorretos[] = "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //char tentativa[] = "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    //printf("jogo 0 = 5? %d ", jogo[0] == '5');
    //printf("%d", valoresCorretos);
    clock_t startTempo, endTempo;
    double tempoResolver;
    startTempo = clock();
    //resolveJogo(jogo,solucao);
    //resolverJogoCompleto(jogosEsolucoes[0].jogo, jogosEsolucoes[0].solucao, 0);
    resolverJogoParcial(jogosEsolucoes[0].jogo, jogosEsolucoes[0].solucao, 0);
    
    endTempo = clock();
    tempoResolver = (double)(endTempo - startTempo) / CLOCKS_PER_SEC;
    printf("Tempo para resolver: %f seconds\n", tempoResolver);
    //imprimirTabuleiro(jogo);
    return 0;
}