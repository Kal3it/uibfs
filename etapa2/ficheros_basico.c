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

    //free(sb); //todo: como gestionarlo?

    return status;
}

int initMB(unsigned int nbloques) {
    sb = malloc(sizeof(struct superbloque));
    unsigned char * buf=malloc(BLOCKSIZE);
    memset(buf,0,BLOCKSIZE);

    if(bread(posSB,sb) == -1){
        return -1;
    } // todo: lo mismo que antes, se puede optimizar

    // Los bloques del mapa de bits se limpian con 0s
    for (int i = sb->posPrimerBloqueMB; i < sb->posUltimoBloqueMB; ++i) {
        if(bwrite(i, buf) == -1){
            return -1;
        }
    }

    /**
     * Se establecen a ocupados los bloques usados por el superbloque, mapa de bits y array de inodos.
     */
    escribit_bit(0,1); // El superbloque es bloque ocupado
    for (int i = sb->posPrimerBloqueMB; i < sb->posUltimoBloqueMB; ++i) {
        escribit_bit(i,1);
    }
    for (int i = sb->posPrimerBloqueAI; i < sb->posUltimoBloqueAI; ++i) {
        escribit_bit(i,1);
    }

    //free(sb); todo: fallo
    free(buf);

    return 0;
}

int initAI(unsigned int ninodos) {
    sb = malloc(sizeof(struct superbloque));
    unsigned int countInodo = 0;
	unsigned int maxInodosEnBloque = BLOCKSIZE / T_INODO;

    if(bread(posSB,sb) == -1){
        return -1;
    } // todo: lo mismo que antes, se puede optimizar

//    if(sizeof(inodo_t) != 128){
//        puts("Problemas de alineacion de estructuras.");
//        return -1;
//    }// todo: debugging

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

    //free(inodos); TODO: refactorizar para que inodos sea visible desde la funcion
    //free(sb); todo: fallo

	return 0;
}

int escribit_bit(unsigned int nbloque, unsigned int bit){
    sb = malloc(sizeof(struct superbloque)); // todo: necesario cargar sb cada vez? se puede optimizar!!
    unsigned char * bufferMB = malloc(BLOCKSIZE);
    unsigned char mascara = 128; // 10000000

    if(bread(posSB,sb) == -1){
        return -1;
    }

    // Operaciones para localizar el bit, byte y bloque
    unsigned int bitAbsoluto = nbloque;
    unsigned int byteAbsoluto = bitAbsoluto / 8;
    unsigned int bitRelativo = bitAbsoluto % 8;
    unsigned int byteRelativo = byteAbsoluto % BLOCKSIZE;
    unsigned int bloqueRelativo = byteAbsoluto / BLOCKSIZE;
    unsigned int bloqueAbsoluto = sb->posPrimerBloqueMB + bloqueRelativo;

    /** Debugging */
    if(nbloque >= sb->totBloques){
        // todo: Voy a añadir estas lineas por si de aqui en adelante tenemos errores,
        // para saber si vienen de aqui.
        // Cuando se entrege eliminamos las lineas.
        fprintf(stderr,"El bloque %u no existe. El ultimo bloque es el %u.\n",nbloque,sb->totBloques-1);
        return -1;
    }

    if(bread(bloqueAbsoluto,bufferMB) == -1){ // Se carga el valor actual del bloque en el mapa de bits
        return -1;
    }

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    if(bit){
        //Para poner un bit a 1:
        bufferMB[byteRelativo] |= mascara; // operador OR para bits
    } else {
        //Para poner un bit a 0:
        bufferMB[byteRelativo] &= ~mascara; // operadores AND y NOT para bits
    }

    /** Debugging */
//    printf("nbloque: %u\n",nbloque);
//    printf("bit absoluto: %u\n",bitAbsoluto);
//    printf("byte absoluto: %u\n",byteAbsoluto);
//    printf("bloque absoluto: %u (desde %u)\n",bloqueAbsoluto,sb->posPrimerBloqueMB);
//    printf("bit relativo: %u\n",bitRelativo);
//    printf("byte relativo: %u\n",byteRelativo);

    if(bwrite(bloqueAbsoluto, bufferMB) == -1){
        return -1;
    }

    free(bufferMB);
    //free(sb); todo: fallo

    return 0;
}

unsigned char leer_bit(unsigned int nbloque){
    sb = malloc(sizeof(struct superbloque));
    unsigned char * bufferMB = malloc(BLOCKSIZE);
    unsigned char mascara = 128; // 10000000

    if(bread(posSB,sb) == -1){
        return -1;
    }  // todo: lo mismo que antes, se puede optimizar

    // Operaciones para localizar el bit, byte y bloque
    unsigned int bitAbsoluto = nbloque;
    unsigned int byteAbsoluto = bitAbsoluto / 8;
    unsigned int bitRelativo = bitAbsoluto % 8;
    unsigned int byteRelativo = byteAbsoluto % BLOCKSIZE;
    unsigned int bloqueRelativo = byteAbsoluto / BLOCKSIZE;
    unsigned int bloqueAbsoluto = sb->posPrimerBloqueMB + bloqueRelativo;

    if(bread(bloqueAbsoluto,bufferMB) == -1){ // todo: Si hay error que devolvemos?? -1 no es un unsigned char
        return -1;
    }

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    mascara &= bufferMB[byteRelativo]; // operador AND para bits
    mascara >>= (7-bitRelativo); // desplazamiento de bits a la derecha

    //free(sb); todo: fallo
    free(bufferMB);

    return mascara;
}

int reservar_bloque(){
    sb = malloc(sizeof(struct superbloque));
    unsigned char * bufferMB = malloc(BLOCKSIZE);
    unsigned char * bufferAux = malloc(BLOCKSIZE);

    if(bread(posSB,sb) == -1){
        return -1;
    } // todo: lo mismo que antes, se puede optimizar

    if(sb->cantBloquesLibres == 0){
        fprintf(stderr,"No quedan bloques libres\n");
        return -1;
    }

    // Analizamos el bloque en busca de la posicion del primer bit a 0
    int posbit = -1, posbyte = -1, posbloque = -1, bitFound = 0;
    memset(bufferAux, 1, BLOCKSIZE);

    for (posbloque = sb->posPrimerBloqueMB; !bitFound ; ++posbloque) {
        bread(posbloque,bufferMB);
        if(memcmp(bufferAux,bufferMB,BLOCKSIZE)) {
            for (posbyte = 0; !bitFound; ++posbyte) {
                unsigned char byte = bufferMB[posbyte];
                if (byte < 255) {
                    for (posbit = 0; !bitFound; ++posbit) {
                        bitFound = !(byte & 128);
                        byte <<= 1;
                    }
                }
            }
        }
    }
    --posbloque;
    --posbyte;
    --posbit;
    --(sb->cantBloquesLibres);
    unsigned int nbloque = ((posbloque - sb->posPrimerBloqueMB) * BLOCKSIZE) + (posbyte * 8) + posbit;

    // todo: debbugging
//    printf("El primer bit a cero esta en la posicion %d del byte %d del bloque %d\n",posbit,posbyte,posbloque);
//    printf("El bloque libre es el %u\n",nbloque);

    // Actualizamos SB
    if(bwrite(posSB,sb) == -1){
        return -1;
    }

    // Marcamos el bloque como ocupado
    if(escribit_bit(nbloque,1) == -1){
        return -1;
    }

    // Limpiamos el bloque con 0s
    memset(bufferAux,0,BLOCKSIZE);
    if(bwrite(nbloque,bufferAux)){
        return -1;
    }

    //free(sb); todo: fallo
    free(bufferMB);
    free(bufferAux);

    return nbloque;
}

int liberar_bloque(unsigned int nbloque){
    sb = malloc(sizeof(struct superbloque));

    if(bread(posSB,sb) == -1){
        return -1;
    } // todo: lo mismo que antes, se puede optimizar

    if(escribit_bit(nbloque,0) == -1){
        return -1;
    }

    ++(sb->cantBloquesLibres);
    if(bwrite(posSB,sb) == -1){
        return -1;
    }

    //free(sb) todo: fallo

    return nbloque;
}

//int  escribir_inodo(struct inodo *inodo,unsigned int ninodo){
//
//    unsigned struct inodo bufferAI[BLOCKSIZE/TAM_INODO];
//    // Cuantos numero de bloque se guardan en 1 bloque del MB? 1024*8 = BLOCKSIZE * 8
//    unsigned int bloqueAI =(ninodo/BLOCKSIZE)+sb->posPrimerBloqueAI;
//    if(bread(bloqueAI,bufferAI)==-1){
//        return -1;
//    }
//    bufferAI[ninodo%(BLOCKSIZE/TAM_INODO)]=inodo;
//    bwrite(bloqueAI,bufferAI);
//
//}
//
//int struct inodo leer_inodo(unsigned int ninodo, struct inodo *inodo){
//
//    unsigned int bloqueAI =(ninodo/BLOCKSIZE)+sb->posPrimerBloqueAI;
//    unsigned inodo bufferAI[BLOCKSIZE/TAM_INODO];
//    if(bread(bloqueAI,bufferAI)==-1){
//        return -1;
//    }
//    inodo = bufferAI[ninodo%(BLOCKSIZE/TAM_INODO)];
//    return inodo;
//
//}
//
//int reservar_inodo(unsigned char tipo, unsigned char permisos){
//
//    if (sb->cantInodosLibres == 0){
//        return -1;
//    }
//
//
// }
