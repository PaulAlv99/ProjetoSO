#include "config.h"

const char* getTempo() {
    static char buffer[TEMPO_TAMANHO];
    time_t now = time(NULL);
    strftime(buffer, sizeof(buffer) - 1, "%d-%m-%Y %H:%M:%S", localtime(&now));
    return buffer;
}