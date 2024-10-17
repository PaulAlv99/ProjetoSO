#include "ficheiro.h"
#define CAPACIDADE_CONFSERVER 1
#define PATH_SIZE 16

struct ServidorConfig{
    char ficheiroJogosCaminho[PATH_SIZE];
    char ficheiroSolucoesCaminho[PATH_SIZE];
};