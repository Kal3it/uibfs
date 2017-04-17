#include "ficheros_basico.h"
// todo: diferenciar entre errores recuperables y no recuperables

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
    unsigned char buf[BLOCKSIZE];
    memset(buf, 0, BLOCKSIZE);

    bread(posSB, &sb);

    // Los bloques del mapa de bits se limpian con 0s
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueMB; ++i) bwrite(i, buf);// todo: Ya se han limpiado en MI_MKFS..?

    // Se establecen a ocupados los bloques usados por el superbloque, mapa de bits y array de inodos.
    escribit_bit(0, 1); // El superbloque es bloque ocupado
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueMB; ++i) escribit_bit(i, 1);
    for (int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; ++i) escribit_bit(i, 1);

    return 0;
}

int initAI(unsigned int ninodos) {
    struct superbloque sb;
    unsigned int maxInodosEnBloque = BLOCKSIZE / T_INODO;
    unsigned int iteraciones, inodosRestantes, inodosEnBloque;
    inodo_t inodos[maxInodosEnBloque];
    bread(posSB, &sb);

    for (unsigned int i = sb.posPrimerBloqueAI, countInodo = 0; i <= sb.posUltimoBloqueAI; ++i) {

        iteraciones = i - (sb.posPrimerBloqueAI),
        inodosRestantes = sb.totInodos - (iteraciones) * (maxInodosEnBloque),
        inodosEnBloque = (inodosRestantes >= maxInodosEnBloque) ? maxInodosEnBloque : inodosRestantes;

        memset(inodos, 0, BLOCKSIZE); // todo Se limpia el bloque con 0, pero ya esta limpiado con MI MKFS?

        for (int j = sb.posPrimerInodoLibre; j < inodosEnBloque; ++j) {

            inodos[j].tipo = 'l'; // todo. tipo l es libre??
            inodos[j].atime = time(NULL);
            inodos[j].mtime = time(NULL);
            inodos[j].ctime = time(NULL);
            inodos[j].nlinks = 0;
            inodos[j].numBloquesOcupados = 0;
            inodos[j].permisos = 7;
            inodos[j].tamEnBytesLog = 0;
            // todo: si sb.primerInodoLibre no fuese 0 habria problemas con la j y el buffer "inodos". es necesario leer el sb?

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
    unsigned char bufferMB[BLOCKSIZE], mascara = 128; // 10000000

    bread(posSB, &sb);

    // Operaciones para localizar el bit, byte y bloque
    unsigned int
            bitAbsoluto = nbloque,
            byteAbsoluto = bitAbsoluto / 8,
            bitRelativo = bitAbsoluto % 8,
            byteRelativo = byteAbsoluto % BLOCKSIZE,
            bloqueRelativo = byteAbsoluto / BLOCKSIZE,
            bloqueAbsoluto = sb.posPrimerBloqueMB + bloqueRelativo;

    // todo Debugging
    if(leer_bit(nbloque) == 1 && bit){
        fprintf(stderr,"El bloque %u ya esta reservado\n",nbloque);
        exit(-1);
    }

    // todo Debugging
    if (nbloque >= sb.totBloques) {
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

    bwrite(bloqueAbsoluto, bufferMB);

    return 0;
}

unsigned char leer_bit(unsigned int nbloque) {
    struct superbloque sb;
    unsigned char bufferMB[BLOCKSIZE], mascara = 128; // 10000000

    bread(posSB, &sb);

    // Operaciones para localizar el bit, byte y bloque
    unsigned int bitAbsoluto = nbloque,
            byteAbsoluto = bitAbsoluto / 8,
            bitRelativo = bitAbsoluto % 8,
            byteRelativo = byteAbsoluto % BLOCKSIZE,
            bloqueRelativo = byteAbsoluto / BLOCKSIZE,
            bloqueAbsoluto = sb.posPrimerBloqueMB + bloqueRelativo;

    bread(bloqueAbsoluto, bufferMB);

    mascara >>= bitRelativo; // desplazamiento de bits a la derecha
    mascara &= bufferMB[byteRelativo]; // operador AND para bits
    mascara >>= (7 - bitRelativo); // desplazamiento de bits a la derecha

    return mascara;
}

int reservar_bloque() {
    struct superbloque sb;
    unsigned char bufferMB[BLOCKSIZE], bufferAux[BLOCKSIZE];

    bread(posSB, &sb);

    if (sb.cantBloquesLibres == 0) {
        fprintf(stderr, "No quedan bloques libres\n");
        exit(NO_QUEDAN_BLOQUES_LIBRES);
    }

    // Analizamos el bloque en busca de la posicion del primer bit a 0
    unsigned int posbit, posbyte, posbloque, bitFound = 0;
    memset(bufferAux, 255, BLOCKSIZE);

    for (posbloque = sb.posPrimerBloqueMB; !bitFound; ++posbloque) {
        bread(posbloque, bufferMB);
        if (memcmp(bufferAux, bufferMB, BLOCKSIZE) != 0) { // Si el bloque del MB tiene algun 0, si es diferente de un buffer lleno de 1s
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

    unsigned int nbloque = 0;
    nbloque += (posbloque - sb.posPrimerBloqueMB) * (BLOCKSIZE * 8); // Desplazamiento en bloques
    nbloque += posbyte * 8; // Desplazamiento en bytes
    nbloque += posbit; // Desplazamiento en bits

    // Actualizamos SB
    bwrite(posSB, &sb);

    // Marcamos el bloque como ocupado
    escribit_bit(nbloque, 1);

    // Limpiamos el bloque con 0s
    memset(bufferAux, 0, BLOCKSIZE);
    bwrite(nbloque, bufferAux);

    return nbloque;
}

int liberar_bloque(unsigned int nbloque) {
    struct superbloque sb;

    bread(posSB, &sb);

    // todo: Debugging
    if(leer_bit(nbloque) == 0){
        fprintf(stderr,"El bloque %u ya esta liberado.\n",nbloque);
        return BLOQUE_FISICO_YA_LIBERADO;
    }

    fprintf(stderr,"Se libera el bloque %u.\n",nbloque);
    escribit_bit(nbloque, 0);

    ++(sb.cantBloquesLibres);
    bwrite(posSB, &sb);

    return nbloque;
}

int escribir_inodo(inodo_t inodo, unsigned int ninodo) {
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE / T_INODO;

    struct superbloque sb;
    inodo_t bufferAI[INODOS_EN_BLOQUE];
    unsigned int nbloqueAI;

    bread(posSB, &sb);

    nbloqueAI = sb.posPrimerBloqueAI + ninodo / INODOS_EN_BLOQUE;

    bread(nbloqueAI, bufferAI);

    bufferAI[ninodo % INODOS_EN_BLOQUE] = inodo;

    bwrite(nbloqueAI, bufferAI);

    return 0;
}

int leer_inodo(unsigned int ninodo, inodo_t *inodo) {
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE / T_INODO;
    struct superbloque sb;
    unsigned int nbloqueAI;
    inodo_t bufferAI[INODOS_EN_BLOQUE];

    bread(posSB, &sb);

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
        fprintf(stderr, "No quedan inodos libres.\n");
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
    for (int i = 0; i < 12; ++i) inodo.punterosDirectos[i] = 0;
    for (int i = 0; i < 3; ++i) inodo.punterosIndirectos[i] = 0;

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

/*
 *
 * CABECERAS DE FUNCIONES RECURSIVAS
 *
 */

/**
 * @param ptrBloqueIndice Numero de bloque fisico que contiene la tabla indice
 * @param indirecciones_restantes Niveles restantes hasta llegar a la tabla indice
 * @param nblogico_relativo Numero de bloque logico dentro de la tabla indice que contiene el numero del siguiente bloque
 * @param denominador
 * @param inodo Puntero a la variable inodo que se irá actualizando durante las llamadas
 * @param reservar Si es 1, en caso de que el bloque no este reservado, se reserva
 * @return Devuelve el numero de bloque fisico de datos a partir del puntero a la tabla indice y el numero de niveles
 */
int indireccionar(unsigned int ptrBloqueIndice, int indirecciones_restantes, unsigned int nblogico_relativo, int denominador, inodo_t * inodo, char reservar);
/**
 * Libera los bloques apuntados mediante tablas indice y libera los bloques que contienen tablas indice en caso de ser necesario.
 * @param ptrBloqueIndice
 * @param indirecciones_restantes
 * @param nblogico_relativo
 * @param denominador
 * @param inodo
 * @return
 */
int liberar_bloques_recursivo(unsigned int ptrBloqueIndice, int indirecciones_restantes, unsigned int nblogico_relativo, int denominador, inodo_t * inodo);
/**
 * Libera el bloque logico $nblogico del inodo $inodo.
 * Si no esta inicializado, omite la liberación pero no devuelve ningún error.
 * @param inodo
 * @param nblogico
 * @return
 */
int liberar_bloque_inodo(inodo_t * inodo, unsigned int nblogico);


/*
 *
 * IMPLEMENTACIONES DE FUNCIONES RECURSIVAS
 *
 */

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
            return BLOQUE_LOGICO_NO_INICIALIZADO;
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

                escribir_inodo(inodo, ninodo);

            } else {
                return BLOQUE_LOGICO_NO_INICIALIZADO;
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
                return (BLOQUE_LOGICO_NO_INICIALIZADO);

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
                return (BLOQUE_LOGICO_NO_INICIALIZADO);
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
                return (BLOQUE_LOGICO_NO_INICIALIZADO);
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

    escribir_inodo(inodo,ninodo);

    return nbloqueFisico;
}

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
    unsigned int ultimoBlogico = (inodo.tamEnBytesLog-1) / BLOCKSIZE;

    fprintf(stderr,"primer BL: %u, ultimo BL: %u.\n",nblogico,ultimoBlogico);
    for (unsigned int i = nblogico; i <= ultimoBlogico ; ++i) {
        liberar_bloque_inodo(&inodo, i);
    }

    escribir_inodo(inodo,ninodo);

    return 0;
}

int liberar_inodo(unsigned int ninodo){
    struct superbloque sb;
    inodo_t inodo;

    liberar_bloques_inodo(ninodo, 0);

    leer_inodo(ninodo,&inodo);
    bread(posSB,&sb);

    // todo debugging
    if(inodo.tipo == 'l'){ // todo: que letra hay que usar?
        fprintf(stderr,"El inodo %u ya es un inodo libre.\n",ninodo);
        exit(INODO_YA_LIBERADO);
    }

    inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
    inodo.tipo = 'l';
    sb.posPrimerInodoLibre = ninodo;
    sb.cantInodosLibres++;

    escribir_inodo(inodo,ninodo);
    bwrite(posSB,&sb);

    return ninodo;
}