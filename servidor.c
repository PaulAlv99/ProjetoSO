#include "config.h"
#define CONFIGFILE "servidor.conf"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Erro: Nome do ficheiro de configuracao nao fornecido.\n");
        return 1;
    }

	if (strcmp(argv[1], CONFIGFILE) != 0)
	{
		printf("Nome do ficheiro de configuracao incorreto. %s\n", argv[1]);
		return 1;
	}
}