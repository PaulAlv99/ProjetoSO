#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>

#define TEMPO_TAMANHO 22
#define BUF_SIZE 2048
// 81 + \0
#define NUMEROS_NO_JOGO 81
#define NUM_LINHAS 9
#define NUM_COLUNAS 9
#define NUM_JOGOS 3
#define INFO_SIZE 256
sem_t semaforoAguardaResposta;
sem_t semAguardar;
// globais
const char *getTempo();
const char *getTempoHoraMinutoSegundo();
void enviarMensagens();
void receberMensagens(int nUtilizadores);
int fecharFicheiro(FILE *file);
FILE *abrirFicheiro(char *filename);
int validarNomeFile(char *arquivoNome, char *padrao);
void err_dump(char *msg);
int readline(int fd, char *ptr, int maxlen);
int writen(int fd, char *ptr, int nbytes);
