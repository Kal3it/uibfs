#include "ficheros_basico.h"

struct superbloque * sb; // TODO: FALTA HACER EL FREE EN ALGUNA PARTE

int tamMB (unsigned int nbloques){
    /**
     * Cuantos bits necesitamos para representar los bloques?
     * 1 bloque -> 1 bit
     * nbloques/8 -> bytes para representar todos los bloques
     * cantidad de bytes / el tamaño en bytes de cada bloque -> numero de bloques!!
     * si la cantidad de bytes no es un multiplo del tamaño en bytes de cada bloque,
     * hay que reservar un bloque mas (que se quedará con una parte sin usar)
     */
	return ((nbloques / 8) % BLOCKSIZE) ? ((nbloques / 8) / BLOCKSIZE) + 1 : ((nbloques / 8) / BLOCKSIZE);
}

int tamAI (unsigned int ninodos){
    /* No existe la posibilidad de saber cuantos inodos van a usar todo el espacio del disco
     * por el hecho de que ese número depende de qué tamaño tendrán los inodos que se crearán en el futuro.
     * Por ejemplo, un usuario puede crear 2 inodos y éstos ocupar el 100% del espacio del disco. Por lo tanto,
     * lo óptimo en este caso sería que la tabla de inodos fuese de 2 punteros.
     *
     * Por otra parte, es posible que el usuario cree un inodo por bloque. En este caso, lo óptimo sería que
     * la tabla de inodos tuviese tantos punteros como bloques.
     *
     * En general, ni una opción ni la otra son las más optimas, por lo tanto heurísticamente se puede considerar
     * que la tabla deberia apuntar a nodos/4.
     */
    return (ninodos * T_INODO) % BLOCKSIZE  ? ((ninodos * T_INODO) / BLOCKSIZE) + 1 : ((ninodos * T_INODO) / BLOCKSIZE);
}

int initSB(unsigned int nbloques, unsigned int ninodos){

    sb = malloc(sizeof(struct superbloque));

    /**
     * Hay que tener en cuenta que los valores sb->posUltima* representan el bloque posterior al ultimo bloque asociado.
     * Por ejemplo, si el primer bloque MB es 1 y el ultimo es 12, los bloques del MB son del 1 al 11.
     */

    sb->posPrimerBloqueMB = posSB + 1; // El bloque #0 lo utiliza el SB, por lo tanto sería el siguiente bloque
    sb->posUltimoBloqueMB = sb->posPrimerBloqueMB + tamMB(nbloques);

    sb->posPrimerBloqueAI = sb->posUltimoBloqueMB;
    sb->posUltimoBloqueAI = sb->posPrimerBloqueAI + tamAI(ninodos);

    sb->posPrimerBloqueDatos = sb->posUltimoBloqueAI;
    sb->posUltimoBloqueDatos = nbloques;

    sb->posInodoRaiz = 0;
    sb->posPrimerInodoLibre = 0;

    sb->cantBloquesLibres = sb->posUltimoBloqueDatos - sb->posPrimerBloqueDatos;
    sb->cantInodosLibres = ninodos;

    sb->totBloques = nbloques;
    sb->totInodos = ninodos;

    int status = bwrite(posSB, sb);

    return status;
}

int initMB(unsigned int nbloques) {

    unsigned char * buf=malloc(BLOCKSIZE);
    memset(buf,0,BLOCKSIZE);

    // Los bloques del mapa de bits se limpian con 0s
    for (int i = sb->posPrimerBloqueMB; i < sb->posUltimoBloqueMB; ++i) {
        if(bwrite(i, buf) == -1){
            return -1;
        }
    }

    free(buf);

    return 0;
}

int initAI(unsigned int ninodos) {

    unsigned int countInodo = 0;
	unsigned int maxInodosEnBloque = BLOCKSIZE / T_INODO;

//    if(sizeof(inodo_t) != 128){
//        puts("Problemas de alineacion de estructuras.");
//        return -1;
//    }

    for (int i = sb->posPrimerBloqueAI; i < sb->posUltimoBloqueAI; ++i) {

        unsigned int
                iteraciones = i - (sb->posPrimerBloqueAI), // TODO: hacer este calculo o ir restando cada iteracion?
                inodosRestantes = sb->totInodos - (iteraciones) * (maxInodosEnBloque),
                inodosEnBloque = (inodosRestantes >= maxInodosEnBloque) ? maxInodosEnBloque : inodosRestantes;

        inodo_t inodos[inodosEnBloque];
        //memset(inodos, 0, size * T_INODO); // TODO: Conseguir que el espacio sin ocupar del array almacene todo 0s?

        for (int j = sb->posPrimerInodoLibre; j < inodosEnBloque; ++j) {

            inodos[j].tipo = 'l';
            inodos[j].atime = time(NULL);
            inodos[j].mtime = time(NULL);
            inodos[j].ctime = time(NULL);
            inodos[j].nlinks = 0;
            inodos[j].numBloquesOcupados = 0;
            inodos[j].permisos = 0xFF;
            inodos[j].tamEnBytesLog = 0;

            // Si no es el ultimo, apunta al siguiente. Si es el ultimo, no apunta a nada.
            inodos[j].punterosDirectos[0] = (countInodo < sb->totInodos-1) ? countInodo + 1 : UINT_MAX ;
            ++countInodo;
        }

        if(bwrite(i, inodos) == -1){
            return -1;
        }
    }

	return 0;
}