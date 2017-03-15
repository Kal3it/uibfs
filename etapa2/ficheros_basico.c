#include "ficheros_basico.h"

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
    /* No existe la posibilidad de saber cuantos inodos van a usar t0do el espacio del disco
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

    struct superbloque * sb = malloc(sizeof(struct superbloque));

    /**
     * Hay que tener en cuenta que los valores sb->posUltima* representan el bloque posterior al ultimo bloque asociado.
     * Por ejemplo, si el primer bloque MB es 1 y el ultimo es 12, los bloques del MB son del 1 al 11.
     */

    sb->posPrimerBloqueMB = posSB + 1; // El bloque #0 lo utiliza el SB, por lo tanto sería el siguiente bloque
    sb->posUltimoBloqueMB = sb->posPrimerBloqueMB + tamMB(nbloques) - 1;

    sb->posPrimerBloqueAI = sb->posUltimoBloqueMB + 1; // todo: Ajustar valor
    sb->posUltimoBloqueAI = sb->posPrimerBloqueAI + tamAI(ninodos) - 1;

    sb->posPrimerBloqueDatos = sb->posUltimoBloqueAI + 1;
    sb->posUltimoBloqueDatos = nbloques - 1;

    sb->posInodoRaiz = 0;
    sb->posPrimerInodoLibre = 0;

    sb->cantBloquesLibres = sb->posUltimoBloqueDatos - sb->posPrimerBloqueDatos + 1;
    sb->cantInodosLibres = ninodos;

    sb->totBloques = nbloques;
    sb->totInodos = ninodos;

    if(bwrite(posSB,sb) == -1){
        return -1;
    }

    free(sb);

    return 0;
}

int initMB(unsigned int nbloques) {
    struct superbloque * sb = malloc(sizeof(struct superbloque)); // TODO: Se puede optimizar?
    unsigned char * buf=malloc(BLOCKSIZE);
    memset(buf,0,BLOCKSIZE);

    if(bread(posSB,sb) == -1){
        return -1;
    }

    // Los bloques del mapa de bits se limpian con 0s
    for (int i = sb->posPrimerBloqueMB; i <= sb->posUltimoBloqueMB; ++i) {
        if(bwrite(i, buf) == -1){
            return -1;
        }
    }

    /**
     * Se establecen a ocupados los bloques usados por el superbloque, mapa de bits y array de inodos.
     */
    escribit_bit(0,1); // El superbloque es bloque ocupado
    for (int i = sb->posPrimerBloqueMB; i <= sb->posUltimoBloqueMB; ++i) {
        escribit_bit(i,1);
    }
    for (int i = sb->posPrimerBloqueAI; i <= sb->posUltimoBloqueAI; ++i) {
        escribit_bit(i,1);
    }

    free(sb);
    free(buf);

    return 0;
}

int initAI(unsigned int ninodos) {
    struct superbloque * sb = malloc(sizeof(struct superbloque));
    unsigned int countInodo = 0;
	unsigned int maxInodosEnBloque = BLOCKSIZE / T_INODO;

    if(bread(posSB,sb) == -1){
        return -1;
    }

    if(sizeof(inodo_t) != 128){
        puts("Problemas de alineacion de estructuras.");
        return -1;
    }// todo: debugging

    for (int i = sb->posPrimerBloqueAI; i <= sb->posUltimoBloqueAI; ++i) {

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
            inodos[j].permisos = 7;
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
    free(sb);

	return 0;
}

int escribit_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque * sb = malloc(sizeof(struct superbloque));
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
    free(sb);

    return 0;
}

unsigned char leer_bit(unsigned int nbloque){
    struct superbloque * sb = malloc(sizeof(struct superbloque));
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

    if(bread(bloqueAbsoluto,bufferMB) == -1){ // todo: Si hay error que devolvemos?? -1 no es un unsigned char
        return -1;
    }

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    mascara &= bufferMB[byteRelativo]; // operador AND para bits
    mascara >>= (7-bitRelativo); // desplazamiento de bits a la derecha

    free(sb);
    free(bufferMB);

    return mascara;
}

int reservar_bloque(){
    struct superbloque * sb = malloc(sizeof(struct superbloque));
    unsigned char * bufferMB = malloc(BLOCKSIZE);
    unsigned char * bufferAux = malloc(BLOCKSIZE);

    if(bread(posSB,sb) == -1){
        return -1;
    }

    if(sb->cantBloquesLibres == 0){
        fprintf(stderr,"No quedan bloques libres\n");
        return -1;
    }

    // Analizamos el bloque en busca de la posicion del primer bit a 0
    int posbit = -1, posbyte = -1, posbloque = -1, bitFound = 0;
    memset(bufferAux, 255, BLOCKSIZE); // TODO: testear

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

//    // todo: debbugging
//    printf("El primer bit a cero esta en la posicion %d del byte %d del bloque %d\n",posbit,posbyte,posbloque);
//    printf("El bloque libre es el %u\n",nbloque);
//
    free(sb);
    free(bufferMB);
    free(bufferAux);

    return nbloque;
}

int liberar_bloque(unsigned int nbloque){
    struct superbloque *  sb = malloc(sizeof(struct superbloque));

    if(bread(posSB,sb) == -1){
        return -1;
    }

    if(escribit_bit(nbloque,0) == -1){
        return -1;
    }

    ++(sb->cantBloquesLibres);
    if(bwrite(posSB,sb) == -1){
        return -1;
    }

    free(sb);

    return nbloque;
}

int escribir_inodo(inodo_t inodo,unsigned int ninodo){
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE/T_INODO;

    struct superbloque * sb = malloc(sizeof(struct superbloque));
    inodo_t * bufferAI = malloc(INODOS_EN_BLOQUE * sizeof(inodo_t));
    unsigned int nbloqueAI;

    if(bread(posSB,sb) == -1){
        return -1;
    }

    nbloqueAI = sb->posPrimerBloqueAI + ninodo/INODOS_EN_BLOQUE;

    if(bread(nbloqueAI,bufferAI) == -1){
        return -1;
    }

    bufferAI[ninodo%INODOS_EN_BLOQUE] = inodo;

    if(bwrite(nbloqueAI,bufferAI) == -1){
        return -1;
    }

    free(sb);
    free(bufferAI);

    return 0;
}

int leer_inodo(unsigned int ninodo, inodo_t * inodo){
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE/T_INODO;
    struct superbloque * sb = malloc(sizeof(struct superbloque));
    unsigned int nbloqueAI;
    inodo_t bufferAI[INODOS_EN_BLOQUE];

    if(bread(posSB,sb) == -1){
        return -1;
    }

    nbloqueAI = sb->posPrimerBloqueAI + ninodo/INODOS_EN_BLOQUE;

    if(bread(nbloqueAI,bufferAI) == -1){
        return -1;
    }

    *inodo = bufferAI[ninodo%INODOS_EN_BLOQUE];

    free(sb);
    //free(bufferAI); // TODO: Hay que hacer free a todo menos al sitio donde apunta inodo!!

    return 0;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque * sb = malloc(sizeof(struct superbloque));
    inodo_t inodo, inodoLibre;

    if(bread(posSB,sb) == -1){
        return -1;
    }

    if(sb->cantInodosLibres == 0){
        fprintf(stderr,"No quedan inodos libres.");
        return -1;
    }

    // Inicializamos el nuevo inodo
    unsigned int t = time(NULL);
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = t;
    inodo.mtime = t;
    inodo.ctime = t;
    inodo.numBloquesOcupados = 0;
    for (int i = 0; i < 12; ++i) {
        inodo.punterosDirectos[i] = 0;
    }
    for (int j = 0; j < 3; ++j) {
        inodo.punterosIndirectos[j] = 0;
    }

    unsigned int posNuevoInodo = sb->posPrimerInodoLibre;

    // Actualizamos el primer inodo libre
    if(leer_inodo(sb->posPrimerInodoLibre,&inodoLibre) == -1){
        return -1;
    }
    sb->posPrimerInodoLibre = inodoLibre.punterosDirectos[0];

    // Guardamos el nuevo inodo
    if(escribir_inodo(inodo,posNuevoInodo) == -1){
        return -1;
    }

    --(sb->cantInodosLibres);

    if(bwrite(posSB, sb) == -1){ // Guardamos la posicion del primer inodo libre y la cantidad de inodos libres actualizada
        return -1;
    }

    free(sb);

    return posNuevoInodo;

 }

int obtener_nrangoBL (inodo_t inodo, unsigned int nblogico, int * ptr){
    if(nblogico < DIRECTOS){
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0){
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1){
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    } else if (nblogico < INDIRECTOS2) {
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr,"El bloque %u esta fuera de rango.",nblogico);
        return -1;
    }
}

int obtener_indice(int nblogico, int nivel_punteros){
    if(nblogico < DIRECTOS){
        return nblogico;

    } else if(nblogico < INDIRECTOS1){
        if(nivel_punteros == 2){
            return (nblogico - INDIRECTOS0)/NPUNTEROS;
        } else if(nivel_punteros == 1){
            return (nblogico-INDIRECTOS0) % NPUNTEROS;
        }

    } else if(nblogico < INDIRECTOS2){
        if(nivel_punteros == 3){
            return (nblogico-INDIRECTOS1)/(NPUNTEROS*NPUNTEROS);
        } else if (nivel_punteros == 2){
            return ((nblogico-INDIRECTOS1) % (NPUNTEROS*NPUNTEROS))/NPUNTEROS;
        } else if (nivel_punteros == 1){
            return ((nblogico-INDIRECTOS1) % (NPUNTEROS*NPUNTEROS)) % NPUNTEROS;
        }
    }

    fprintf(stderr,"Bloque (%u) fuera de rango o nivel_punteros invalido (%u)",nblogico,nivel_punteros);
    return -1;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    return 0;
}