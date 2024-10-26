#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../include/cjson/cJSON.h"

#define TEMPO_TAMANHO 22
#define BUF_SIZE 256
// 81 + \0
#define NUMEROS_NO_JOGO 81
#define NUM_LINHAS 9
#define NUM_COLUNAS 9
#define NUM_JOGOS 5
#define NUM_CLIENTES 100
// globais
const char *getTempo();