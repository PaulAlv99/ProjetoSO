#include "../headers/servidor.h"

struct Servidor servidor;
struct Jogo jogosEsolucoes[5];

// Função para carregar as configurações do servidor
void carregarConfigServidor(char *nomeFicheiro)
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
        strtok(buffer, "\n");
        strcpy(servidor.ficheiroJogosESolucoesCaminho, buffer);
        // printf("caminho: %s", serverConfig.ficheiroJogosESolucoesCaminho);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.portaServidor = atoi(buffer);
        // printf("porta: %d", serverConfig.portaServidor);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.numeroConexoes = atoi(buffer);
        // printf("numeroConexoes: %d", serverConfig.numeroConexoes);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.tamanhoTabuleiro = atoi(buffer);
        // printf("tamanhoTabuleiro: %d", serverConfig.tamanhoTabuleiro);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.numeroLinhas = atoi(buffer);
        // printf("numerLinhas: %d", serverConfig.numeroLinhas);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.numeroColunas = atoi(buffer);
        // printf("numeroColunas: %d", serverConfig.numeroColunas);
    }
    if (fgets(buffer, PATH_SIZE, config) != NULL)
    {
        strtok(buffer, "\n");
        servidor.numeroJogos = atoi(buffer);
        // printf("numeroJogos: %d", serverConfig.numeroJogos);
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