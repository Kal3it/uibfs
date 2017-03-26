#include "ficheros_basico.h"
// todo: en vez de if(bla == -1) return -1, hacer system exit
// todo: diferenciar entre errores recuperables y no recuperables
// todo: ademas de con los sb, cambiar memoria estatica por dinamica para los buffer

int tamMB(unsigned int nbloques) {
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

int tamAI(unsigned int ninodos) {
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
    return (ninodos * T_INODO) % BLOCKSIZE ? ((ninodos * T_INODO) / BLOCKSIZE) + 1 : ((ninodos * T_INODO) / BLOCKSIZE);
}

int initSB(unsigned int nbloques, unsigned int ninodos) {

    struct superbloque sb;

    sb.posPrimerBloqueMB = posSB + 1; // El bloque #0 lo utiliza el SB, por lo tanto sería el siguiente bloque
    sb.posUltimoBloqueMB = sb.posPrimerBloqueMB + tamMB(nbloques) - 1;

    sb.posPrimerBloqueAI = sb.posUltimoBloqueMB + 1;
    sb.posUltimoBloqueAI = sb.posPrimerBloqueAI + tamAI(ninodos) - 1;

    sb.posPrimerBloqueDatos = sb.posUltimoBloqueAI + 1;
    sb.posUltimoBloqueDatos = nbloques - 1;

    sb.posInodoRaiz = 0;
    sb.posPrimerInodoLibre = 0;

    sb.cantBloquesLibres = sb.posUltimoBloqueDatos - sb.posPrimerBloqueDatos + 1;
    sb.cantInodosLibres = ninodos;

    sb.totBloques = nbloques;
    sb.totInodos = ninodos;

    bwrite(posSB, &sb);

    return 0;
}

int initMB(unsigned int nbloques) {
    struct superbloque sb;
    unsigned char *buf = malloc(BLOCKSIZE);
    memset(buf, 0, BLOCKSIZE);

    bread(posSB, &sb);

    // Los bloques del mapa de bits se limpian con 0s
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueMB; ++i) {
        bwrite(i, buf);
    }

    /**
     * Se establecen a ocupados los bloques usados por el superbloque, mapa de bits y array de inodos.
     */
    escribit_bit(0, 1); // El superbloque es bloque ocupado
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueMB; ++i) {
        escribit_bit(i, 1);
    }
    for (int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; ++i) {
        escribit_bit(i, 1);
    }

    free(buf);

    return 0;
}

int initAI(unsigned int ninodos) {
    struct superbloque sb;
    unsigned int countInodo = 0;
    unsigned int maxInodosEnBloque = BLOCKSIZE / T_INODO;

    bread(posSB, &sb);

//    if (sizeof(inodo_t) != 128) {
//        puts("Problemas de alineacion de estructuras.");
//        return -1;
//    }// todo: debugging

    for (int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; ++i) {

        unsigned int
                iteraciones = i - (sb.posPrimerBloqueAI), // TODO: hacer este calculo o ir restando cada iteracion?
                inodosRestantes = sb.totInodos - (iteraciones) * (maxInodosEnBloque),
                inodosEnBloque = (inodosRestantes >= maxInodosEnBloque) ? maxInodosEnBloque : inodosRestantes;

        inodo_t inodos[inodosEnBloque];
        //memset(inodos, 0, size * T_INODO); // ºTODO: Conseguir que el espacio sin ocupar del array almacene todo 0s?

        for (int j = sb.posPrimerInodoLibre; j < inodosEnBloque; ++j) {

            inodos[j].tipo = 'l';
            inodos[j].atime = time(NULL);
            inodos[j].mtime = time(NULL);
            inodos[j].ctime = time(NULL);
            inodos[j].nlinks = 0;
            inodos[j].numBloquesOcupados = 0;
            inodos[j].permisos = 7;
            inodos[j].tamEnBytesLog = 0;

            // Si no es el ultimo, apunta al siguiente. Si es el ultimo, no apunta a nada.
            inodos[j].punterosDirectos[0] = (countInodo < sb.totInodos - 1) ? countInodo + 1 : UINT_MAX;
            ++countInodo;
        }

        bwrite(i, inodos);
    }

    return 0;
}

int escribit_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque sb;
    unsigned char *bufferMB = malloc(BLOCKSIZE);
    unsigned char mascara = 128; // 10000000

    bread(posSB, &sb);

    // Operaciones para localizar el bit, byte y bloque
    unsigned int bitAbsoluto = nbloque;
    unsigned int byteAbsoluto = bitAbsoluto / 8;
    unsigned int bitRelativo = bitAbsoluto % 8;
    unsigned int byteRelativo = byteAbsoluto % BLOCKSIZE;
    unsigned int bloqueRelativo = byteAbsoluto / BLOCKSIZE;
    unsigned int bloqueAbsoluto = sb.posPrimerBloqueMB + bloqueRelativo;

    /** Debugging */
    if (nbloque >= sb.totBloques) {
        // todo: Voy a añadir estas lineas por si de aqui en adelante tenemos errores,
        // para saber si vienen de aqui.
        // Cuando se entrege eliminamos las lineas.
        fprintf(stderr, "El bloque %u no existe. El ultimo bloque es el %u.\n", nbloque, sb.totBloques - 1);
        exit(BLOQUE_FUERA_DE_RANGO);
    }

    bread(bloqueAbsoluto, bufferMB);// Se carga el valor actual del bloque en el mapa de bits

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    if (bit) {
        //Para poner un bit a 1:
        bufferMB[byteRelativo] |= mascara; // operador OR para bits
    } else {
        //Para poner un bit a 0:
        bufferMB[byteRelativo] &= ~mascara; // operadores AND y NOT para bits
    }

    // todo Debugging
//    printf("nbloque: %u\n",nbloque);
//    printf("bit absoluto: %u\n",bitAbsoluto);
//    printf("byte absoluto: %u\n",byteAbsoluto);
//    printf("bloque absoluto: %u (desde %u)\n",bloqueAbsoluto,sb.posPrimerBloqueMB);
//    printf("bit relativo: %u\n",bitRelativo);
//    printf("byte relativo: %u\n",byteRelativo);

    bwrite(bloqueAbsoluto, bufferMB);
    free(bufferMB);

    return 0;
}

unsigned char leer_bit(unsigned int nbloque) {
    struct superbloque sb;
    unsigned char *bufferMB = malloc(BLOCKSIZE);
    unsigned char mascara = 128; // 10000000

    bread(posSB, &sb);

    // Operaciones para localizar el bit, byte y bloque
    unsigned int bitAbsoluto = nbloque;
    unsigned int byteAbsoluto = bitAbsoluto / 8;
    unsigned int bitRelativo = bitAbsoluto % 8;
    unsigned int byteRelativo = byteAbsoluto % BLOCKSIZE;
    unsigned int bloqueRelativo = byteAbsoluto / BLOCKSIZE;
    unsigned int bloqueAbsoluto = sb.posPrimerBloqueMB + bloqueRelativo;

    bread(bloqueAbsoluto, bufferMB);

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    mascara &= bufferMB[byteRelativo]; // operador AND para bits
    mascara >>= (7 - bitRelativo); // desplazamiento de bits a la derecha

    free(bufferMB);

    return mascara;
}

int reservar_bloque() {
    struct superbloque sb;
    unsigned char *bufferMB = malloc(BLOCKSIZE);
    unsigned char *bufferAux = malloc(BLOCKSIZE);

    bread(posSB, &sb);

    if (sb.cantBloquesLibres == 0) {
        fprintf(stderr, "No quedan bloques libres\n");
        exit(NO_QUEDAN_BLOQUES_LIBRES);
    }

    // Analizamos el bloque en busca de la posicion del primer bit a 0
    int posbit = -1, posbyte = -1, posbloque = -1, bitFound = 0;
    memset(bufferAux, 255, BLOCKSIZE);

    for (posbloque = sb.posPrimerBloqueMB; !bitFound; ++posbloque) {
        bread(posbloque, bufferMB);
        if (memcmp(bufferAux, bufferMB, BLOCKSIZE)) {
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
    --(sb.cantBloquesLibres);
    unsigned int nbloque = ((posbloque - sb.posPrimerBloqueMB) * BLOCKSIZE) + (posbyte * 8) + posbit;

    // Actualizamos SB
    bwrite(posSB, &sb);

    // Marcamos el bloque como ocupado
    escribit_bit(nbloque, 1);

    // Limpiamos el bloque con 0s
    memset(bufferAux, 0, BLOCKSIZE);
    bwrite(nbloque, bufferAux);

//    // todo: debbugging
//    printf("El primer bit a cero esta en la posicion %d del byte %d del bloque %d\n",posbit,posbyte,posbloque);
//    printf("El bloque libre es el %u\n",nbloque);
//
    free(bufferMB);
    free(bufferAux);

    return nbloque;
}

int liberar_bloque(unsigned int nbloque) {
    struct superbloque sb;

    bread(posSB, &sb);

    // todo: Debugging
    if(leer_bit(nbloque) == 0){
        fprintf(stderr,"El bloque %u ya esta liberado.\n",nbloque);
        exit(-1);
    }

    escribit_bit(nbloque, 0);

    ++(sb.cantBloquesLibres);
    bwrite(posSB, &sb);

    return nbloque;
}

int escribir_inodo(inodo_t inodo, unsigned int ninodo) {
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE / T_INODO;

    struct superbloque sb;
    inodo_t *bufferAI = malloc(INODOS_EN_BLOQUE * sizeof(inodo_t));
    unsigned int nbloqueAI;

    bread(posSB, &sb);

    nbloqueAI = sb.posPrimerBloqueAI + ninodo / INODOS_EN_BLOQUE;

    bread(nbloqueAI, bufferAI);

    bufferAI[ninodo % INODOS_EN_BLOQUE] = inodo;

    bwrite(nbloqueAI, bufferAI);

    free(bufferAI);

    return 0;
}

int leer_inodo(unsigned int ninodo, inodo_t *inodo) {
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE / T_INODO;
    struct superbloque sb;
    unsigned int nbloqueAI;
    inodo_t bufferAI[INODOS_EN_BLOQUE];

    if (bread(posSB, &sb) == -1) {
        return -1;
    }

    nbloqueAI = sb.posPrimerBloqueAI + ninodo / INODOS_EN_BLOQUE;

    bread(nbloqueAI, bufferAI);

    *inodo = bufferAI[ninodo % INODOS_EN_BLOQUE];

    return 0;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque sb;
    inodo_t inodo, inodoLibre;

    bread(posSB, &sb);

    if (sb.cantInodosLibres == 0) {
        fprintf(stderr, "No quedan inodos libres.");
        exit(NO_QUEDAN_INODOS_LIBRES);
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

    /**
     * Actualizamos la lista enlazada de inodos libres:
     * 1. Recuperamos el puntero al siguiente inodo libre almacenado en sb.posPrimerInodoLibre.
     * 2. Recuperamos el ninodo del inodo libre.
     * 3. Actualizamos el atributo sb.posPrimerInodoLibre a partir del puntero al siguiente inodo libre
     * 4. Ahora podemos guardar el inodo en la posicion almacenada en el paso 2.
     * 5. Decrementamos la cantidad de inodos libres.
     * 6. Actualizamos el superbloque
     */
    leer_inodo(sb.posPrimerInodoLibre, &inodoLibre);
    unsigned int posNuevoInodo = sb.posPrimerInodoLibre;
    sb.posPrimerInodoLibre = inodoLibre.punterosDirectos[0];

    // Guardamos el nuevo inodo
    escribir_inodo(inodo, posNuevoInodo);

    --(sb.cantInodosLibres);

    bwrite(posSB, &sb);

    return posNuevoInodo;
}

///**
// * Establece en $ptr el puntero al bloque con el cual, directa o indirectamente, se llega al bloque de datos.
// * Devuelve el numero de niveles que hay que descender para llegar al bloque de datos.
// *
// * @param inodo Inodo que contiene el bloque lógico al que se desea acceder
// * @param nblogico Número de bloque lógico al que se quiere acceder
// * @param ptr Puntero a la tabla de punteros del siguiente nivel
// * @return Número de niveles que hay que descender para llegar al bloque de datos
// */
//int obtener_nrangoBL(inodo_t inodo, unsigned int nblogico, int *ptr) {
//    if (nblogico < DIRECTOS) {
//        *ptr = inodo.punterosDirectos[nblogico];
//        return 0;
//    } else if (nblogico < INDIRECTOS0) {
//        *ptr = inodo.punterosIndirectos[0];
//        return 1;
//    } else if (nblogico < INDIRECTOS1) {
//        *ptr = inodo.punterosIndirectos[1];
//        return 2;
//    } else if (nblogico < INDIRECTOS2) {
//        *ptr = inodo.punterosIndirectos[2];
//        return 3;
//    } else {
//        *ptr = 0;
//        fprintf(stderr, "El bloque %u esta fuera de rango.", nblogico);
//        fprintf(stderr, "Rango [%u, %u].",0,INDIRECTOS2);
//        return -1;
//    }
//}
//
/**
 * Devuelve la posicion
 * @param nblogico
 * @param nivel_punteros
 * @return
 */
//int obtener_indice(int nblogico, int nivel_punteros) {
//    if (nblogico < DIRECTOS) {
//        return nblogico;
//
//    } else if (nblogico < INDIRECTOS1) {
//        if (nivel_punteros == 2) {
//            return (nblogico - INDIRECTOS0) / NPUNTEROS;
//        } else if (nivel_punteros == 1) {
//            return (nblogico - INDIRECTOS0) % NPUNTEROS;
//        }
//
//    } else if (nblogico < INDIRECTOS2) {
//        if (nivel_punteros == 3) {
//            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
//        } else if (nivel_punteros == 2) {
//            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
//        } else if (nivel_punteros == 1) {
//            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
//        }
//    }
//
//    fprintf(stderr, "Bloque (%u) fuera de rango o nivel_punteros invalido (%u)", nblogico, nivel_punteros);
//    return -1;
//}
//
//int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar) {
//    inodo_t inodo;
//    int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice, buffer[NPUNTEROS];
//
//    if (leer_inodo(ninodo, &inodo) == -1) {
//        return -1;
//    }
//
//    ptr = 0;
//    ptr_ant = 0;
//    salvar_inodo = 0;
//    nRangoBL = obtener_nrangoBL(inodo, nblogico, &ptr);
//    nivel_punteros = nRangoBL;
//
//    while (nivel_punteros > 0) {
//        if (ptr == 0) { // Significa que ese bloque logico del inodo aun no está reservado
//
//            if (reservar == 0) {
//                return -1; // error
//
//            } else {
//                salvar_inodo = 1;
//                ptr = reservar_bloque();
//                if (ptr == -1) {
//                    return -1;
//                }
//                inodo.numBloquesOcupados++;
//                inodo.ctime = time(NULL);
//                if (nivel_punteros == nRangoBL) {
//                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
//
//                } else {
//                    buffer[indice] = ptr;
//                    if (bwrite(ptr_ant, buffer) == -1) {
//                        return -1;
//                    }
//
//                }
//            }
//        }
//
//        if (bread(ptr, buffer) == -1) {
//            return -1;
//        }
//        indice = obtener_indice(nblogico, nivel_punteros);
//        ptr_ant = ptr;
//        ptr = buffer[indice];
//        --nivel_punteros;
//
//        printf("nblogico: %d\n",nblogico);
//        printf("ptr bloque físico de punteros nivel %d: %d\n",nivel_punteros, ptr);
//        printf("indice nivel %d: %d\n",nivel_punteros,);
//        printf("nRangoBL: %d\n",nRangoBL);
//
//    }
//
//    if (ptr == 0) {
//        if (reservar == 0) {
//            return -1; //error
//        } else {
//            salvar_inodo = 1;
//            ptr = reservar_bloque();
//            ++inodo.numBloquesOcupados;
//            inodo.ctime = time(NULL);
//            if (nRangoBL == 0) {
//                inodo.punterosDirectos[nblogico] = ptr;
//            } else {
//                buffer[indice] = ptr;
//                if (bwrite(ptr_ant, buffer) == -1) {
//                    return -1;
//                }
//            }
//        }
//        if (salvar_inodo == 1) {
//            //escribir_inodo(inodo, ninodo);
//        }
//    }
//
//    return ptr;
//}

int indireccionar(unsigned int ptrBloqueIndice, int indirecciones_restantes, unsigned int nblogico_relativo, int denominador, inodo_t * inodo, char reservar){

    unsigned int bufferBloqueIndice[NPUNTEROS], indice = nblogico_relativo/denominador;

    bread(ptrBloqueIndice,&bufferBloqueIndice);

    if(bufferBloqueIndice[indice] == 0){
        if(reservar){
            bufferBloqueIndice[indice] = reservar_bloque();
            ++inodo->numBloquesOcupados;
            inodo->ctime = time(NULL);
            bwrite(ptrBloqueIndice,&bufferBloqueIndice);
        } else {
            fprintf(stderr,"Se ha intentado acceder a un bloque logico que no esta inicializado.\n");
            exit(BLOQUE_LOGICO_NO_INICIALIZADO);
        }
    }

    if(indirecciones_restantes > 1){

        return indireccionar(
                bufferBloqueIndice[indice],
                indirecciones_restantes - 1,
                nblogico_relativo % denominador,
                denominador / NPUNTEROS,
                inodo,
                reservar);

    } else {
        return bufferBloqueIndice[indice]; // Ultima indireccion
    }
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    inodo_t inodo;
    unsigned int nblogico_relativo, ptrInicial, denominador;
    int nivel;

    leer_inodo(ninodo, &inodo);

    if(nblogico < DIRECTOS){

        // Reservar si procede
        if(inodo.punterosDirectos[nblogico] == 0){
            if(reservar){
                inodo.punterosDirectos[nblogico] = reservar_bloque();
                ++inodo.numBloquesOcupados;
                inodo.ctime = time(NULL);

                if((inodo.tamEnBytesLog / BLOCKSIZE) < (BLOCKSIZE * (nblogico+1))){ // todo: es correcta la operacion?
                    //printf("El nuevo tamaño del fichero es %u\n",BLOCKSIZE * (nblogico+1)); // todo: Si es demasiado grande no lo coge bien
                    inodo.tamEnBytesLog = BLOCKSIZE * (nblogico+1);
                }

                escribir_inodo(inodo, ninodo);

            } else {
                fprintf(stderr,"Se ha intentado acceder a un bloque logico que no esta inicializado.\n");
                exit(BLOQUE_LOGICO_NO_INICIALIZADO);

            }
        }

        return inodo.punterosDirectos[nblogico]; // No hace falta indireccionar

    } else if(nblogico < DIRECTOS + NPUNTEROS){

        // Reservar si procede
        if(inodo.punterosIndirectos[0] == 0){
            if(reservar){
                inodo.punterosIndirectos[0] = reservar_bloque();
                ++inodo.numBloquesOcupados;
                inodo.ctime = time(NULL);
                escribir_inodo(inodo, ninodo);

            } else {
                fprintf(stderr,"Se ha intentado acceder a un bloque logico que no esta inicializado.\n");
                exit(BLOQUE_LOGICO_NO_INICIALIZADO);

            }
        }

        nblogico_relativo = nblogico - DIRECTOS;
        ptrInicial = inodo.punterosIndirectos[0];
        nivel = 1;
        denominador = 1;

    } else if(nblogico < DIRECTOS + NPUNTEROS + NPUNTEROS_CUADRADO){

        // Reservar si procede
        if(inodo.punterosIndirectos[1] == 0){
            if(reservar){
                inodo.punterosIndirectos[1] = reservar_bloque();
                ++inodo.numBloquesOcupados;
                inodo.ctime = time(NULL);
                escribir_inodo(inodo, ninodo);
            } else {
                fprintf(stderr,"Se ha intentado acceder a un bloque logico que no esta inicializado.\n");
                exit(BLOQUE_LOGICO_NO_INICIALIZADO);
            }
        }

        nblogico_relativo = nblogico - DIRECTOS - NPUNTEROS;
        ptrInicial = inodo.punterosIndirectos[1];
        nivel = 2;
        denominador = NPUNTEROS;

    } else if(nblogico < DIRECTOS + NPUNTEROS + NPUNTEROS_CUADRADO + NPUNTEROS_CUBO){

        // Reservar si procede
        if(inodo.punterosIndirectos[2] == 0){
            if(reservar){
                inodo.punterosIndirectos[2] = reservar_bloque();
                ++inodo.numBloquesOcupados;
                inodo.ctime = time(NULL);
                escribir_inodo(inodo, ninodo);
            } else {
                fprintf(stderr,"Se ha intentado acceder a un bloque logico que no esta inicializado.\n");
                exit(BLOQUE_LOGICO_NO_INICIALIZADO);
            }
        }

        nblogico_relativo = nblogico - DIRECTOS - NPUNTEROS - NPUNTEROS_CUADRADO;
        ptrInicial = inodo.punterosIndirectos[2];
        nivel = 3;
        denominador = NPUNTEROS_CUADRADO;

    } else {
        fprintf(stderr,"El bloque logico %u esta fuera del rango.\n",nblogico);
        exit(BLOQUE_LOGICO_FUERA_DE_RANGO);

    }

    unsigned int nbloqueFisico = indireccionar(ptrInicial, nivel, nblogico_relativo, denominador, &inodo, reservar);

    if((inodo.tamEnBytesLog / BLOCKSIZE) < (BLOCKSIZE * (nblogico+1))){
        //printf("El nuevo tamaño del fichero es %u\n",BLOCKSIZE * (nblogico+1)); todo: debug
        inodo.tamEnBytesLog = BLOCKSIZE * (nblogico+1);
    }

    escribir_inodo(inodo,ninodo);

    return nbloqueFisico;
}

/**
 * Libera los bloques apuntados mediante tablas indice y libera los bloques que contienen tablas indice en caso de ser necesario.
 * @param ptrBloqueIndice
 * @param indirecciones_restantes
 * @param nblogico_relativo
 * @param denominador
 * @param inodo
 * @return
 */
int liberar_bloques_recursivo(unsigned int ptrBloqueIndice, int indirecciones_restantes, unsigned int nblogico_relativo, int denominador, inodo_t * inodo){

    unsigned int bufferBloqueIndice[NPUNTEROS],
            bufferBloqueIndiceApuntado[NPUNTEROS],
            bufferAux[NPUNTEROS],
            indice = nblogico_relativo/denominador,
            liberar = 0;

    bread(ptrBloqueIndice,&bufferBloqueIndice);

    // Comprobamos si la entrada apunta a algun bloque
    if(bufferBloqueIndice[indice] == 0){
        return 0;
    }

    if(indirecciones_restantes > 1){

        liberar_bloques_recursivo(
                bufferBloqueIndice[indice],
                indirecciones_restantes - 1,
                nblogico_relativo % denominador,
                denominador / NPUNTEROS,
                inodo);

        // En este punto tenemos la tabla indice que apunta a otra tabla indice
        bread(bufferBloqueIndice[indice], &bufferBloqueIndiceApuntado);
        memset(bufferAux,0,NPUNTEROS * sizeof(unsigned int));

        // Se comprueba si la tabla indice contiene algun puntero aun
        liberar = (memcmp(bufferBloqueIndiceApuntado,bufferAux,(NPUNTEROS * sizeof(unsigned int)))) ? 0 : 1;

    } else {
        // En este punto tenemos la tabla indice que apunta al bloque de datos
        liberar = 1;
    }

    if(liberar){
        liberar_bloque(bufferBloqueIndice[indice]);

        bufferBloqueIndice[indice] = 0;
        bwrite(ptrBloqueIndice, bufferBloqueIndice);

        --inodo->numBloquesOcupados;
        inodo->ctime = time(NULL);

    }

    return 0;
}

/**
 * Libera el bloque logico $nblogico del inodo $inodo.
 * Si no esta inicializado, omite la liberación pero no devuelve ningún error.
 * @param inodo
 * @param nblogico
 * @return
 */
int liberar_bloque_inodo(inodo_t * inodo, unsigned int nblogico) {
    unsigned int nblogico_relativo, ptrInicial, denominador;
    int nivel;

    if (nblogico < DIRECTOS) {

        if(inodo->punterosDirectos[nblogico] == 0){
            return 0;
        }

        liberar_bloque(inodo->punterosDirectos[nblogico]);
        inodo->punterosDirectos[nblogico] = 0;
        --inodo->numBloquesOcupados;
        inodo->ctime = time(NULL);

        return 0;

    } else if (nblogico < DIRECTOS + NPUNTEROS) {

        nblogico_relativo = nblogico - DIRECTOS;
        ptrInicial = inodo->punterosIndirectos[0];
        nivel = 1;
        denominador = 1;

    } else if (nblogico < DIRECTOS + NPUNTEROS + NPUNTEROS_CUADRADO) {

        nblogico_relativo = nblogico - DIRECTOS - NPUNTEROS;
        ptrInicial = inodo->punterosIndirectos[1];
        nivel = 2;
        denominador = NPUNTEROS;

    } else if (nblogico < DIRECTOS + NPUNTEROS + NPUNTEROS_CUADRADO + NPUNTEROS_CUBO) {

        nblogico_relativo = nblogico - DIRECTOS - NPUNTEROS - NPUNTEROS_CUADRADO;
        ptrInicial = inodo->punterosIndirectos[2];
        nivel = 3;
        denominador = NPUNTEROS_CUADRADO;

    } else {
        fprintf(stderr, "El bloque logico %u esta fuera del rango.\n", nblogico);
        return (BLOQUE_LOGICO_FUERA_DE_RANGO);

    }

    // Comprobamos que el puntero apunte a algun bloque
    if(ptrInicial == 0){
        return 0;
    }

    liberar_bloques_recursivo(ptrInicial, nivel, nblogico_relativo, denominador, inodo);

    // Comprobamos si hay que liberar el bloque que contiene la tabla indice
    unsigned int bufferBloqueIndiceApuntado[NPUNTEROS], bufferAux[NPUNTEROS];
    bread(inodo->punterosIndirectos[nivel - 1], &bufferBloqueIndiceApuntado);
    memset(bufferAux, 0, NPUNTEROS * sizeof(unsigned int));

    if (memcmp(&bufferBloqueIndiceApuntado, &bufferAux, (NPUNTEROS * sizeof(unsigned int))) == 0) {
        liberar_bloque(inodo->punterosIndirectos[nivel - 1]);
        inodo->punterosIndirectos[nivel - 1] = 0;
        --inodo->numBloquesOcupados;
        inodo->ctime = time(NULL);
    }

    return 0;
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int nblogico){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);

    for (unsigned int i = nblogico; i < (inodo.tamEnBytesLog / BLOCKSIZE); ++i) {
        liberar_bloque_inodo(&inodo, i);
    }

    // todo: nuevo tamEnBytesLog??

    escribir_inodo(inodo,ninodo);

    return 0;
}

int liberar_inodo(unsigned int ninodo){
    struct superbloque sb;
    inodo_t inodo;

    // todo: falta comprobar si el inodo ya esta libre o no.

    liberar_bloques_inodo(ninodo, 0);

    leer_inodo(ninodo,&inodo);
    bread(posSB,&sb);

    if(inodo.tipo == 'x'){
        fprintf(stderr,"El inodo %u ya es un inodo libre.\n",ninodo);
        return (INODO_YA_LIBERADO);
    }

    inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
    inodo.tipo = 'x';
    sb.posPrimerInodoLibre = ninodo;
    sb.cantInodosLibres++;

    escribir_inodo(inodo,ninodo);
    bwrite(posSB,&sb);

    return ninodo;
}