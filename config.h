#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>

//1KB
#define BUF_SIZE 128
#define IP_SIZE 16  
#define CAPACIDADE_CONFIG 100
// Definição da struct para armazenar as configurações
typedef struct {
    int idJogo;
    char jogoAtual[BUF_SIZE];
    char solucaoJogo[BUF_SIZE];
} Servidor;

typedef struct{
    int idCliente;
    char ipServidor[IP_SIZE];
}Cliente;