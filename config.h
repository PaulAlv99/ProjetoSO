#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>

#define TEMPO_TAMANHO 22

//globais
const char* getTempo() {
    static char buffer[TEMPO_TAMANHO];
    time_t now = time(NULL);
    strftime(buffer, sizeof(buffer) - 1, "%d-%m-%Y %H:%M:%S", localtime(&now));
    return buffer;
}


