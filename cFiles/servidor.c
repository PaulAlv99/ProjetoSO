#include "../headers/servidor.h"

// structs
struct Jogo jogosEsolucoes[NUM_JOGOS];
// globais
static int idCliente = 0;
// tricos
pthread_mutex_t mutexServidorLog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClienteID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNTentativas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexjogosAleatorios = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforoAguardaResposta;

// so tem o ficheiro da localizacao dos jogos e solucoes que neste caso e apenas 1 ficheiro
void carregarConfigServidor(char *nomeFicheiro, struct ServidorConfig *serverConfig)
{
    FILE *config = abrirFicheiro(nomeFicheiro);

    fseek(config, 0, SEEK_END);
    long tamanhoFicheiro = ftell(config);
    rewind(config);

    char buffer[tamanhoFicheiro];
    int contadorConfigs = 0;
    // ambas tem o \0 no final nao esquecer se tiver mais linhas de config convem tirar \n
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strcpy(serverConfig->ficheiroJogosESolucoesCaminho, strtok(buffer, "\n"));
        // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        serverConfig->porta = atoi(strtok(buffer, "\n"));
        // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    }
    contadorConfigs++;
    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(1);
    }

    return;
}

void logEventoServidor(const char *message)
{

    // modo append
    char *ficheiroLogs = "logs/LogServidor.txt";
    pthread_mutex_lock(&mutexServidorLog);
    FILE *file = fopen(ficheiroLogs, "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o ficheiro de log");
        pthread_mutex_unlock(&mutexServidorLog);
        return;
    }
    fprintf(file, "[%s] %s\n", getTempo(), message);
    fclose(file);
    pthread_mutex_unlock(&mutexServidorLog);
}
void logQueEventoServidor(int numero)
{
    switch (numero)
    {
    case 1:
        logEventoServidor("Servidor comecou");
        break;
    case 2:
        logEventoServidor("Servidor parou");
        break;
    case 3:
        // fazer outra funcao para saber ID cliente ex. novo cliente e seu id
        logEventoServidor("O cliente id: conectou-se");
        break;
    case 4:
        // fazer outra funcao para saber ID cliente
        logEventoServidor("Servidor enviou um jogo id cliente:");
        break;
    case 5:
        // fazer outra funcao para saber ID cliente
        logEventoServidor("Servidor recebeu uma solucao.id cliente:");
        break;
    case 6:
        // fazer outra funcao para saber ID cliente
        logEventoServidor("Servidor enviou numero de erradasid cliente:");
        break;
    case 7:
        // fazer outra funcao para saber ID cliente
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

// atualiza os valoresCorretos da Ultima Tentativa
char *atualizaValoresCorretosCompletos(char tentativaAtual[], char valoresCorretos[], char solucao[], int *nTentativas)
{
    char *logCliente = malloc(1024);
    char Tentativas[100];
    sprintf(Tentativas, "Tentativa n: %d\n", *nTentativas);
    // Retornar tenativas para escrever no log do cliente
    for (int i = 0; i < strlen(tentativaAtual); i++)
    {
        if (valoresCorretos[i] == '0')
        {
            if (tentativaAtual[i] == solucao[i])
            {
                valoresCorretos[i] = tentativaAtual[i];
                char message[1024];
                // sprintf(message, "\nValor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
                // strcat(logCliente, message);
                // printf("%s", message);

                // printf("%d \n", valoresCorretos);
            }
            else
            {
                // char message[1024] = "";
                // sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
                // strcat(logCliente, message);
                // printf("%s", message);
            }
        }
    }
    pthread_mutex_lock(&mutexNTentativas);
    *nTentativas = *nTentativas + 1;
    pthread_mutex_unlock(&mutexNTentativas);
    return logCliente;
}

// Atualiza o booleano Resolvido se o jogo tiver sido resolvido
bool verificaResolvido(char valoresCorretos[], char solucao[], bool resolvido)
{
    for (int i = 0; i < strlen(valoresCorretos); i++)
    {
        if (valoresCorretos[i] != solucao[i])
        {
            return false;
        }
    }
    return true;
}

// ResolveJogo
// void resolverJogoCompleto(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){

//         nTentativas = nTentativas + 1;
//         tentarSolucaoCompleta(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosCompletos(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }

// Solucao parcial (Tentar um valor por vez)
// void tentarSolucaoParcial(char tentativaAtual[], char valoresCorretos[])
// {
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if ((tentativaAtual[i] != '0') && (tentativaAtual[i] != valoresCorretos[i]))
//         {
//             char numero = tentativaAtual[i];
//             int numeroInt = (int)(numero);
//             int novoNumero = numeroInt + 1;
//             char novoNumeroChar = (char)(novoNumero);
//             tentativaAtual[i] = novoNumeroChar;
//             break;
//         }
//         else if (tentativaAtual[i] == '0')
//         {
//             tentativaAtual[i] = '1';
//             break;
//         }
//     }
// }

// void atualizaValoresCorretosParcial(char tentativaAtual[], char valoresCorretos[], char solucao[], int nTentativas)
// {
//     char Tentativas[100];
//     sprintf(Tentativas, "Tentativa n: %d \n", nTentativas);
//     logEventoCliente(Tentativas);
//     for (int i = 0; i < strlen(tentativaAtual); i++)
//     {
//         if (valoresCorretos[i] == '0')
//         {
//             if (tentativaAtual[i] == solucao[i])
//             {
//                 valoresCorretos[i] = tentativaAtual[i];
//                 char message[1024];
//                 sprintf(message, "Valor correto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//                 // printf("%d \n", valoresCorretos);
//             }
//             else
//             {
//                 char message[1024];
//                 sprintf(message, "Valor incorreto(%d), na posição %d da String \n", tentativaAtual[i], i + 1);
//                 logEventoCliente(message);
//                 printf(message);
//                 return;
//             }
//         }
//     }
// }

void carregarFicheiroJogosSolucoes(char *nomeFicheiro)
{
    FILE *config = abrirFicheiro(nomeFicheiro);

    char buffer[BUF_SIZE];
    int contadorConfigs = 0;
    while (fgets(buffer, BUF_SIZE, config) != NULL)
    {
        // Leitura do idJogo (primeira linha)
        char *resultado = strtok(buffer, "\n");
        jogosEsolucoes[contadorConfigs].idJogo = atoi(resultado);
        // Leitura do IP do servidor (segunda linha)
        if (fgets(buffer, BUF_SIZE, config) != NULL)
        {
            char *resultado = strtok(buffer, "\n");
            strcpy(jogosEsolucoes[contadorConfigs].jogo, resultado);
            if (fgets(buffer, BUF_SIZE, config) != NULL)
            {
                char *resultado = strtok(buffer, "\n");
                strcpy(jogosEsolucoes[contadorConfigs].solucao, resultado);
            }
        }

        contadorConfigs++; // Contar o número de configurações lidas
    }

    fecharFicheiro(config);
    if (contadorConfigs == 0)
    {
        printf("Sem configs\n");
        exit(1);
    }
    // printf("Configs n:%d\n",contadorConfigs);
    return;
}

// // ResolveJogo
// void resolverJogoParcial(char jogo[], char solucao[], int nTentativas)
// {
//     char tentativaAtual[NUMEROS_NO_JOGO];
//     char valoresCorretos[NUMEROS_NO_JOGO];
//     strncpy(tentativaAtual, jogo, NUMEROS_NO_JOGO);
//     strncpy(valoresCorretos, jogo, NUMEROS_NO_JOGO);
//     bool resolvido;
//     resolvido = false;
//     printf("Jogo Inicial: \n \n");
//     imprimirTabuleiro(jogo);
//     while (!resolvido)
//     {
//         // for(int i = 0; i< 10; i++){
//         nTentativas = nTentativas + 1;
//         tentarSolucaoParcial(tentativaAtual, valoresCorretos);
//         atualizaValoresCorretosParcial(tentativaAtual, valoresCorretos, solucao, nTentativas);
//         resolvido = verificaResolvido(valoresCorretos, solucao, resolvido);

//         printf("tentativaAtual: \n");
//         imprimirTabuleiro(tentativaAtual);

//         printf("ValoresCorretos: \n");
//         imprimirTabuleiro(valoresCorretos);

//         printf("Solução obtida até o momento: \n");
//         imprimirTabuleiro(valoresCorretos);
//     }
//     //}
//     printf("Parabéns, esta é a resolução correta! \n");
//     char TentativasTotais[100];
//     sprintf(TentativasTotais, "Tentativas totais: %d \n", nTentativas);
//     logEventoCliente(TentativasTotais);
//     printf(TentativasTotais);
// }
struct ServidorConfig construtorServer(int dominio, int servico, int protocolo, __u_long interface, int porta, int backlog, char *ficheiroJogosESolucoesCaminho)
{
    struct ServidorConfig servidor = {0};
    servidor.dominio = dominio;
    servidor.servico = servico;
    servidor.protocolo = protocolo;
    servidor.interface = interface;
    servidor.porta = porta;
    servidor.backlog = backlog;
    strcpy(servidor.ficheiroJogosESolucoesCaminho, ficheiroJogosESolucoesCaminho);
    return servidor;
}
void iniciarServidorSocket(struct ServidorConfig *server)
{
    int socketServidor = socket(server->dominio, server->servico, server->protocolo);
    if (socketServidor == -1)
    {
        perror("Erro ao criar socket");
        exit(1);
    }
    struct sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = server->dominio;
    enderecoServidor.sin_port = htons(server->porta);
    enderecoServidor.sin_addr.s_addr = server->interface;
    int bindResult = bind(socketServidor, (struct sockaddr *)&enderecoServidor, sizeof(enderecoServidor));
    if (bindResult == -1)
    {
        perror("Erro ao fazer bind");
        exit(1);
    }
    int listenResult = listen(socketServidor, server->backlog);
    if (listenResult == -1)
    {
        perror("Erro ao fazer listen");
        exit(1);
    }
    printf("==== Servidor Iniciado ====\n");
    printf("====== Porta: %d =======\n\n", server->porta);

    while (1)
    {
        struct sockaddr_in enderecoCliente;
        int tamanhoEndereco = sizeof(enderecoCliente);
        int socketCliente = accept(socketServidor, (struct sockaddr *)&enderecoCliente, (socklen_t *)&tamanhoEndereco);

        if (socketCliente == -1)
        {
            perror("Erro ao aceitar conexão");
            continue;
        }
        char temp[BUF_SIZE] = {0};
        pthread_mutex_lock(&mutexClienteID);
        idCliente++;
        sprintf(temp, "%d|", idCliente);
        pthread_mutex_unlock(&mutexClienteID);

        if (fork() == 0)
        {
            struct ClienteConfig clienteConfig = {0};
            // Processo filho
            close(socketServidor);
            write(socketCliente, temp, BUF_SIZE);
            printf("Cliente-%d conectado\n", idCliente);
            char buffer[BUF_SIZE] = {0};
            // rand time dá sempre mesma seed se chegarem ao mesmo tempo
            srand(getpid());
            int nJogo = rand() % NUM_JOGOS;
            char *jogoADar = jogosEsolucoes[nJogo].jogo;
            receberMensagemETratarServer(buffer, socketCliente, clienteConfig, nJogo, jogoADar);
            close(socketCliente);
            exit(0);
        }
        else
        {
            // Processo pai
            close(socketCliente);
        }
    }
    close(socketServidor);
}
void receberMensagemETratarServer(char *buffer, int socketCliente, struct ClienteConfig clienteConfig, int nJogo, char *jogoADar)
{
    sem_init(&semaforoAguardaResposta, 0, 1);
    while (recv(socketCliente, buffer, BUF_SIZE, 0) > 0)
    {
        printf("Mensagem recebida: %s\n", buffer);
        char *idCliente = strtok(buffer, "|");
        char *tipoJogo = strtok(NULL, "|");
        char *tipoResolucao = strtok(NULL, "|");
        char *temJogo = strtok(NULL, "|");
        char *idJogo = strtok(NULL, "|");
        char *jogo = strtok(NULL, "|");
        char *valoresCorretos = strtok(NULL, "|");
        char *tempoInicio = strtok(NULL, "|");
        char *tempoFinal = strtok(NULL, "|");
        char *resolvido = strtok(NULL, "|");
        char *numeroTentativas = strtok(NULL, "|");
        if (strcmp(temJogo, "SEM_JOGO") == 0)
        {
            if ((idCliente || tipoJogo || tipoResolucao || temJogo || idJogo || jogo || valoresCorretos || tempoInicio || tempoFinal || resolvido || numeroTentativas) == NULL)
            {
                printf("Erro: Mensagem recebida com formato inválido\n");
                break;
            }

            // sem_wait(&semaforoAguardaResposta);

            char *temp = malloc(1024);
            sprintf(temp, "Enviou um jogo para o cliente-%ld", clienteConfig.idCliente);
            strcpy(clienteConfig.tipoJogo, tipoJogo);
            strcpy(clienteConfig.tipoResolucao, tipoResolucao);
            strcpy(clienteConfig.TemJogo, "COM_JOGO");
            strcpy(clienteConfig.jogoAtual.jogo, jogoADar);
            strcpy(clienteConfig.jogoAtual.valoresCorretos, jogoADar);
            strcpy(clienteConfig.jogoAtual.tempoInicio, getTempoHoraMinutoSegundo());
            strcpy(clienteConfig.jogoAtual.tempoFinal, tempoFinal);
            clienteConfig.jogoAtual.resolvido = atoi(resolvido);
            clienteConfig.jogoAtual.numeroTentativas = atoi(numeroTentativas);
            clienteConfig.jogoAtual.idJogo = nJogo;
            clienteConfig.idCliente = atoi(idCliente);

            memset(buffer, 0, BUF_SIZE);
            sprintf(buffer, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d",
                    clienteConfig.idCliente,
                    clienteConfig.tipoJogo,
                    clienteConfig.tipoResolucao,
                    clienteConfig.TemJogo,
                    clienteConfig.jogoAtual.idJogo,
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    clienteConfig.jogoAtual.tempoInicio,
                    clienteConfig.jogoAtual.tempoFinal,
                    clienteConfig.jogoAtual.resolvido,
                    clienteConfig.jogoAtual.numeroTentativas);

            write(socketCliente, buffer, BUF_SIZE);
            // sem_post(&semaforoAguardaResposta);

            printf("Mensagem enviada: %s\n", buffer);
            logEventoServidor(temp);
            free(temp);
            memset(buffer, 0, BUF_SIZE);
        }

        if (strcmp(temJogo, "COM_JOGO") == 0)
        {
            if ((idCliente || tipoJogo || tipoResolucao || temJogo || idJogo || jogo || valoresCorretos || tempoInicio || tempoFinal || resolvido || numeroTentativas) == NULL)
            {
                printf("Erro: Mensagem recebida com formato inválido\n");
                break;
            }

            clienteConfig.idCliente = atoi(idCliente);
            strcpy(clienteConfig.TemJogo, temJogo);
            strcpy(clienteConfig.tipoJogo, tipoJogo);
            strcpy(clienteConfig.tipoResolucao, tipoResolucao);
            clienteConfig.jogoAtual.idJogo = nJogo;
            strcpy(clienteConfig.jogoAtual.jogo, jogo);
            strcpy(clienteConfig.jogoAtual.valoresCorretos, valoresCorretos);
            strcpy(clienteConfig.jogoAtual.tempoInicio, tempoInicio);
            strcpy(clienteConfig.jogoAtual.tempoFinal, tempoFinal);
            clienteConfig.jogoAtual.resolvido = atoi(resolvido);
            clienteConfig.jogoAtual.numeroTentativas = atoi(numeroTentativas);

            char *logCliente;
            atualizaValoresCorretosCompletos(
                clienteConfig.jogoAtual.jogo,
                clienteConfig.jogoAtual.valoresCorretos,
                jogosEsolucoes[clienteConfig.jogoAtual.idJogo].solucao,
                &clienteConfig.jogoAtual.numeroTentativas);

            if (verificaResolvido(
                    clienteConfig.jogoAtual.valoresCorretos,
                    jogosEsolucoes[clienteConfig.jogoAtual.idJogo].solucao,
                    clienteConfig.jogoAtual.resolvido))
            {
                clienteConfig.jogoAtual.resolvido = 1;
            }

            // sem_wait(&semaforoAguardaResposta);
            char *temp = malloc(1024);
            sprintf(temp, "Recebeu uma solução do cliente-%d", clienteConfig.idCliente);

            memset(buffer, 0, BUF_SIZE);
            sprintf(buffer, "%u|%s|%s|%s|%d|%s|%s|%s|%s|%d|%d",
                    clienteConfig.idCliente,
                    clienteConfig.tipoJogo,
                    clienteConfig.tipoResolucao,
                    clienteConfig.TemJogo,
                    clienteConfig.jogoAtual.idJogo,
                    clienteConfig.jogoAtual.jogo,
                    clienteConfig.jogoAtual.valoresCorretos,
                    clienteConfig.jogoAtual.tempoInicio,
                    clienteConfig.jogoAtual.tempoFinal,
                    clienteConfig.jogoAtual.resolvido,
                    clienteConfig.jogoAtual.numeroTentativas);

            write(socketCliente, buffer, BUF_SIZE);
            // sem_post(&semaforoAguardaResposta);

            printf("Mensagem enviada: %s\n", buffer);
            logEventoServidor(temp);
            free(temp);
            memset(buffer, 0, BUF_SIZE);
        }
    }
    char *temp;
    sprintf(temp, "Cliente-%d desconectado", clienteConfig.idCliente);
    printf("%s\n", temp);
}

int main(int argc, char **argv)
{
    struct ServidorConfig serverConfig = {0};

    if (argc < 2)
    {
        printf("Erro: Nome do ficheiro nao fornecido.\n");
        return 1;
    }

    if (strcmp(argv[1], CONFIGFILE) != 0)
    {
        printf("Nome do ficheiro incorreto\n");
        return 1;
    }
    carregarConfigServidor(argv[1], &serverConfig);
    // printf("Caminho jogos e solucoes: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    carregarFicheiroJogosSolucoes(serverConfig.ficheiroJogosESolucoesCaminho);
    serverConfig = construtorServer(AF_INET, SOCK_STREAM, 0, INADDR_ANY, serverConfig.porta, 1, serverConfig.ficheiroJogosESolucoesCaminho);
    logQueEventoServidor(1);
    iniciarServidorSocket(&serverConfig);
    return 0;
}