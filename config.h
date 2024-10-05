#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>

struct servidorConfig{
    int idJogo;
    int solucaoJogo[8][8];
    int jogoAtual[8][8];
};

struct clienteConfig{
    int idCliente;
    char ipServidor;
};