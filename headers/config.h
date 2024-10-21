#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>

#define TEMPO_TAMANHO 22
#define BUF_SIZE 2048
//81 + \0
#define NUMEROS_NO_JOGO 81
#define NUM_LINHAS 9
#define NUM_COLUNAS 9
#define NUM_JOGOS 5
//globais
const char* getTempo();


