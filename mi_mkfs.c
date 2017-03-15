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

    if(bmount(argv[1]) == -1){
        return -1;
    }

	for (int i = 0; i < nbloques; ++i)
	{
		if(bwrite(i,buf) == -1){
            return -1;
        }
	}

	unsigned int ninodos = nbloques/4;
    if(initSB(nbloques, ninodos) == -1){
        return -1;
    }
    if(initMB(nbloques) == -1){
        return -1;
    }
    if(initAI(ninodos) == -1){
        return -1;
    }

    // Creacion del directorio raiz. Se reserva el inodo 0.
    if(reservar_inodo('d',7) == -1){
        return -1;
    }

	if(bumount() == -1){
        return -1;
    }

    free(buf);

    return 0;
}