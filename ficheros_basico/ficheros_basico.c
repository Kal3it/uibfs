#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    return ((nbloques / 8) % BLOCKSIZE) ? ((nbloques / 8) / BLOCKSIZE) + 1 : ((nbloques / 8) / BLOCKSIZE);
}

int tamAI(unsigned int ninodos) {
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

    // Se establecen a ocupados los bloques usados por el superbloque, mapa de bits y array de inodos.
    escribit_bit(0, 1);
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueMB; ++i) escribit_bit(i, 1);
    for (int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; ++i) escribit_bit(i, 1);

    return 0;
}

int initAI(unsigned int ninodos) {
    struct superbloque sb;
    const unsigned int INODOS_EN_BLOQUE = BLOCKSIZE / T_INODO;
    unsigned int iteraciones, inodosRestantes, inodosEnBloque;
    inodo_t inodos[INODOS_EN_BLOQUE];
    bread(posSB, &sb);

    for (unsigned int i = sb.posPrimerBloqueAI, countInodo = 0; i <= sb.posUltimoBloqueAI; ++i) {

        iteraciones = i - (sb.posPrimerBloqueAI),
        inodosRestantes = sb.totInodos - (iteraciones) * (INODOS_EN_BLOQUE),
        inodosEnBloque = (inodosRestantes >= INODOS_EN_BLOQUE) ? INODOS_EN_BLOQUE : inodosRestantes;

        for (int j = sb.posPrimerInodoLibre; j < inodosEnBloque; ++j) {

            inodos[j].tipo = TIPO_LIBRE;
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
    unsigned char bufferMB[BLOCKSIZE], mascara = 128; // 10000000

    bread(posSB, &sb);

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
        exit(-1);
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
        return NO_QUEDAN_BLOQUES_LIBRES;
    }

    // Analizamos el bloque en busca de la posicion del primer bit a 0
    unsigned int posbit, posbyte, posbloque, bitFound = 0;
    memset(bufferAux, 255, BLOCKSIZE);

    for (posbloque = sb.posPrimerBloqueMB; !bitFound; ++posbloque) {
        bread(posbloque, bufferMB);
        if (memcmp(bufferAux, bufferMB, BLOCKSIZE) != 0) {
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

    bwrite(posSB, &sb);

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
        return NO_QUEDAN_INODOS_LIBRES;
    }

    // todo Debugging
    if(permisos > ((unsigned int) 7)){
        fprintf(stderr, "El valor para permisos '%u' es invalido.\n",permisos);
        exit(PERMISOS_INVALIDOS);
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

    escribir_inodo(inodo, posNuevoInodo);

    --(sb.cantInodosLibres);

    bwrite(posSB, &sb);

    return posNuevoInodo;
}

/*
 *
 * CABECERAS DE FUNCIONES RECURSIVAS Y AUXILIARES
 *
 */

int indireccionar(unsigned int *ptrBloqueIndice, int indirecciones_restantes, inodo_t * inodo, unsigned  int nblogico, char reservar);

int liberar_bloques_recursivo(unsigned int *ptrBloqueIndice, int indirecciones_restantes, inodo_t * inodo, unsigned int nblogico);

int liberar_bloque_inodo(inodo_t * inodo, unsigned int nblogico);

int obtener_nrangoBL(inodo_t *inodo, unsigned int nblogico, unsigned int *ptr);

int asignar_ptr_inicial(inodo_t * inodo, unsigned int nblogico, unsigned int ptrInicial);

int obtener_indice(unsigned int nblogico, int nivel_punteros);

/*
 *
 * IMPLEMENTACIONES DE FUNCIONES RECURSIVAS Y AUXILIARES
 *
 */

int obtener_nrangoBL (inodo_t * inodo, unsigned int nblogico, unsigned int *ptr){

    if (nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }else if(nblogico < INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }else if(nblogico < INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }else if(nblogico < INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }else{
        *ptr = 0;
        fprintf(stderr, "Bloque logico '%u' fuera de rango\n",nblogico);
        exit(BLOQUE_LOGICO_FUERA_DE_RANGO);
    }
}

int asignar_ptr_inicial(inodo_t * inodo, unsigned int nblogico, unsigned int ptrInicial){
    if (nblogico < DIRECTOS){
        inodo->punterosDirectos[nblogico] = ptrInicial;
        return 0;
    }else if(nblogico < INDIRECTOS0){
        inodo->punterosIndirectos[0] = ptrInicial;
        return 1;
    }else if(nblogico < INDIRECTOS1){
        inodo->punterosIndirectos[1] = ptrInicial;
        return 2;
    }else if(nblogico < INDIRECTOS2){
        inodo->punterosIndirectos[2] = ptrInicial;
        return 3;
    }else{
        fprintf(stderr, "Bloque logico '%u' fuera de rango\n",nblogico);
        exit(BLOQUE_LOGICO_FUERA_DE_RANGO);
    }
}

int obtener_indice(unsigned int nblogico, int nivel_punteros){
    if (nblogico < DIRECTOS) return nblogico;
    else if (nblogico < INDIRECTOS0) return (nblogico-DIRECTOS);
    else if (nblogico < INDIRECTOS1){

        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1){
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2){

        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS*NPUNTEROS);
        } else if (nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        } else if (nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }

    exit(-1);
}

int indireccionar(unsigned int *ptrBloqueIndice, int indirecciones_restantes, inodo_t * inodo, unsigned  int nblogico, char reservar){

    if(indirecciones_restantes > 0){

        if(*ptrBloqueIndice == 0){
            if(reservar){
                *ptrBloqueIndice = reservar_bloque();
                if(*ptrBloqueIndice == NO_QUEDAN_BLOQUES_LIBRES){
                    return NO_QUEDAN_BLOQUES_LIBRES;
                }
                ++inodo->numBloquesOcupados;
                inodo->ctime = time(NULL);
            } else {
                return BLOQUE_LOGICO_NO_INICIALIZADO;
            }
        }

        unsigned int
                bufferBloqueIndice[NPUNTEROS],
                indice = obtener_indice(nblogico, indirecciones_restantes);

        bread(*ptrBloqueIndice,bufferBloqueIndice);

        char entrada_actualizada = reservar && bufferBloqueIndice[indice] == 0;

        int nBloqueFisico = indireccionar(
                &bufferBloqueIndice[indice],
                indirecciones_restantes - 1,
                inodo,
                nblogico,
                reservar);

        if(nBloqueFisico < 0){ // NO_QUEDAN_BLOQUES_LIBRES o BLOQUE_LOGICO_NO_INICIALIZADO
            return nBloqueFisico;
        }

        if(entrada_actualizada){
            bwrite(*ptrBloqueIndice,bufferBloqueIndice);
        }

        return nBloqueFisico;

    } else {

        if(*ptrBloqueIndice == 0){
            if(reservar){
                *ptrBloqueIndice = reservar_bloque();
                if(*ptrBloqueIndice == NO_QUEDAN_BLOQUES_LIBRES){
                    return NO_QUEDAN_BLOQUES_LIBRES;
                }
                ++inodo->numBloquesOcupados;
                inodo->ctime = time(NULL);
            } else {
                return BLOQUE_LOGICO_NO_INICIALIZADO;
            }
        }

        return *ptrBloqueIndice;
    }
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    inodo_t inodo;
    unsigned int ptrInicial;
    int nbloqueFisico, nivel;

    leer_inodo(ninodo, &inodo);

    nivel = obtener_nrangoBL(&inodo,nblogico,&ptrInicial);
    
	nbloqueFisico = indireccionar(&ptrInicial, nivel, &inodo, nblogico, reservar);
    
	asignar_ptr_inicial(&inodo,nblogico,ptrInicial);

    if(nbloqueFisico == NO_QUEDAN_BLOQUES_LIBRES){
        fprintf(stderr,"No quedan bloques libres\n");
        return NO_QUEDAN_BLOQUES_LIBRES;
    }

    escribir_inodo(inodo,ninodo);

    return nbloqueFisico;
}

int liberar_bloques_recursivo(unsigned int *ptrBloqueIndice, int indirecciones_restantes, inodo_t * inodo, unsigned int nblogico){

    if(indirecciones_restantes > 0){

        unsigned int
                bufferBloqueIndice[NPUNTEROS],
                indice = obtener_indice(nblogico, indirecciones_restantes);

        // Comprobamos si la entrada apunta a algun bloque
        if(*ptrBloqueIndice == 0){
            return 0;
        }

        bread(*ptrBloqueIndice,bufferBloqueIndice);

        liberar_bloques_recursivo(
                &bufferBloqueIndice[indice],
                indirecciones_restantes - 1,
                inodo,
                nblogico);

        bwrite(*ptrBloqueIndice,bufferBloqueIndice);

        // Comprobamos si el bloque indice no contiene mas bloques asignados
		// Si es el caso, se libera
        unsigned int bufferCeros[NPUNTEROS];
        memset(bufferCeros,0,NPUNTEROS * sizeof(unsigned int));

        if(!memcmp(bufferCeros, bufferBloqueIndice, NPUNTEROS * sizeof(unsigned int))){
            liberar_bloque(*ptrBloqueIndice);
            //printf("Liberando bloque indice %u para BL %u\n",*ptrBloqueIndice,nblogico);
            *ptrBloqueIndice = 0;

            --inodo->numBloquesOcupados;
            inodo->ctime = time(NULL);
        }

    } else {

        if(*ptrBloqueIndice != 0) {
            liberar_bloque(*ptrBloqueIndice);
            //printf("Liberando bloque datos %u para BL %u\n",*ptrBloqueIndice,nblogico);
            *ptrBloqueIndice = 0;

            --inodo->numBloquesOcupados;
            inodo->ctime = time(NULL);
        }
    }

    return 0;
}

int liberar_bloque_inodo(inodo_t *inodo, unsigned int nblogico) {
    unsigned int ptrInicial, nivel;

    nivel = obtener_nrangoBL(inodo, nblogico, &ptrInicial);

    liberar_bloques_recursivo(&ptrInicial,nivel,inodo,nblogico);

    asignar_ptr_inicial(inodo, nblogico, ptrInicial);

    return 0;
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int nblogico){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);
    unsigned int ultimoBlogico = (inodo.tamEnBytesLog-1) / BLOCKSIZE;

    if(inodo.tamEnBytesLog == 0) return 0; // No hay bloques que liberar

    //fprintf(stderr,"primer BL: %u, ultimo BL: %u.\n",nblogico,ultimoBlogico);
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
    if(inodo.tipo == TIPO_LIBRE){
        fprintf(stderr,"El inodo %u ya es un inodo libre.\n",ninodo);
        exit(-1);
    }

    inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
    inodo.tipo = TIPO_LIBRE;
    sb.posPrimerInodoLibre = ninodo;
    sb.cantInodosLibres++;

    escribir_inodo(inodo,ninodo);
    bwrite(posSB,&sb);

    return ninodo;
}