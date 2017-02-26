#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "etapa1/bloques.h"
#include "etapa2/ficheros_basico.h"

int main(int argc, char const *argv[])
{
	if(argc != 3){
		puts("Uso: mi_mkfs <nombre_fichero> <cantidad_bloques>");
		return -1;
	}

    unsigned int nbloques = atoi(argv[2]);
    unsigned char *buf = (unsigned char*)malloc(BLOCKSIZE);
	memset(buf,0,BLOCKSIZE);

	bmount(argv[1]);

	for (int i = 0; i < nbloques; ++i)
	{
		bwrite(i,buf);
	}

	unsigned int ninodos = nbloques/4;
    initSB(nbloques, ninodos);
    initMB(nbloques);
    initAI(ninodos);

	bumount();

    free(buf);

    return 0;
}