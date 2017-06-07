#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "bloques/bloques.h"
#include "ficheros_basico/ficheros_basico.h"

int main(int argc, char const *argv[])
{
	if(argc != 3){
		puts("Uso: mi_mkfs <nombre_fichero> <cantidad_bloques>");
		return -1;
	}

    bmount(argv[1]);

    unsigned int nbloques = atoi(argv[2]);
    unsigned char buf[BLOCKSIZE];
	memset(buf,0,BLOCKSIZE);

	for (int i = 0; i < nbloques; ++i) bwrite(i,buf);

	unsigned int ninodos = nbloques/4;
    initSB(nbloques, ninodos);
    initMB(nbloques);
    initAI(ninodos);

    // Creacion del directorio raiz. Se reserva el inodo 0.
    reservar_inodo('d',7);

	bumount();

    return 0;
}