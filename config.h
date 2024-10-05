#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>

//1KB
#define BUF_SIZE 1024

// Definição da struct para armazenar as configurações
typedef struct {
    int idJogo;
    char jogoAtual[BUF_SIZE];
    char solucaoJogo[BUF_SIZE];
} ServidorConfig;

struct ClienteConfig{
    int idCliente;
    char ipServidor;
};