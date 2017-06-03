#include "ficheros.h"

int tiene_permiso(char permisos_inodo, char permiso_comprobado){
    switch (permiso_comprobado){
        case 'r':
            return (permisos_inodo & 4) == 4; //todo: (permisos_inodo & 4) o (permisos_inodo & 4 == 4)
        case 'w':
            return (permisos_inodo & 2) == 2;
        case 'x':
            return (permisos_inodo & 1) == 1;
    }
    fprintf(stderr,"No existe el permiso %c\n",permiso_comprobado);
    exit(-1);
}

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    if(nbytes == 0){
        inodo.mtime = time(NULL);
        escribir_inodo(inodo,ninodo);
        return nbytes;
    }

    if(!tiene_permiso(inodo.permisos, 'w')){
        fprintf(stderr,"Este inodo no tiene permisos de escritura.\n");
        return PERMISOS_INSUFICIENTES;
    }

    if(offset+nbytes-1 > TAM_MAX_FICHERO){
        fprintf(stderr,"Acceso fuera de rango.\n");
        return ACCESO_FUERA_DE_RANGO;
    }

    char buffer[BLOCKSIZE];
    unsigned int
            firstByte = offset,
            lastByte = offset + nbytes - 1,

            firstBloqueLogico = firstByte/BLOCKSIZE,
            lastBloqueLogico = lastByte/BLOCKSIZE,

            fByteRelativo = firstByte % BLOCKSIZE,
            lByteRelativo = lastByte % BLOCKSIZE,

            ptrFirstBloque = traducir_bloque_inodo(ninodo, firstBloqueLogico, 1),
            ptrLastBloque,// = traducir_bloque_inodo(ninodo, lastBloqueLogico, 1),
            bytesEscritos = 0;

    switch (firstBloqueLogico == lastBloqueLogico){
        case 1:
            //fprintf(stderr,"Primer y unico bloque: En el bloque fisico %u desde la posicion del buffer_original %u, escribimos desde el byte del buffer %u hasta la %u (%u bytes)\n",ptrFirstBloque, 0, fByteRelativo, lByteRelativo,lByteRelativo - fByteRelativo + 1);
            bread(ptrFirstBloque, buffer);
            memcpy(buffer + fByteRelativo, buf_original, lByteRelativo - fByteRelativo + 1);
            bwrite(ptrFirstBloque, buffer);
            bytesEscritos += lByteRelativo - fByteRelativo + 1;
            break;

        case 0:
            // Escribimos sobre el primero
            //fprintf(stderr,"Primer bloque (%u): En el bloque fisico %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u (%u bytes)\n",firstBloqueLogico, ptrFirstBloque, bytesEscritos, fByteRelativo, BLOCKSIZE-1,BLOCKSIZE - fByteRelativo);
            bread(ptrFirstBloque, buffer);
            memcpy(buffer + fByteRelativo, buf_original+bytesEscritos, BLOCKSIZE - fByteRelativo);
            bwrite(ptrFirstBloque, buffer);
            bytesEscritos += BLOCKSIZE - fByteRelativo;

            // Escribimos sobre los del medio
            for (int i = firstBloqueLogico+1; i <= lastBloqueLogico-1; ++i) {
                //fprintf(stderr,"Bloque intermedio (%u): En el bloque %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u\n",i, traducir_bloque_inodo(ninodo,i,1), bytesEscritos, 0, BLOCKSIZE-1);
                bwrite(traducir_bloque_inodo(ninodo,i,1),buf_original+bytesEscritos);
                bytesEscritos += BLOCKSIZE;
            }

            // Escribimos sobre el ultimo
            //fprintf(stderr,"Ultimo bloque (%u): En el bloque fisico %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u (%u bytes)\n",lastBloqueLogico, ptrLastBloque, bytesEscritos, 0, lByteRelativo,lByteRelativo+1);
            ptrLastBloque = traducir_bloque_inodo(ninodo, lastBloqueLogico, 1);
            bread(ptrLastBloque, buffer);
            memcpy(buffer, buf_original+bytesEscritos, lByteRelativo+1);
            bwrite(ptrLastBloque, buffer);
            bytesEscritos += lByteRelativo+1;
            break;
    }

    leer_inodo(ninodo, &inodo); // Leemos el inodo actualizado (es posible que se hayan actualizado los punteros a bloques)

    //fprintf(stderr,"Antiguo tamaño: %u\n",inodo.tamEnBytesLog);
    if(lastByte+1 > inodo.tamEnBytesLog){
        inodo.tamEnBytesLog = lastByte+1;
        inodo.ctime = time(NULL);
    }
    //fprintf(stderr,"Nuevo tamaño: %u\n",inodo.tamEnBytesLog);

    inodo.mtime = time(NULL);

    escribir_inodo(inodo,ninodo);

    return bytesEscritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    if(!tiene_permiso(inodo.permisos, 'r')){
        fprintf(stderr,"Este inodo no tiene permisos de lectura.\n");
        return PERMISOS_INSUFICIENTES;
    }

    if(nbytes == 0){
        inodo.atime = time(NULL);
        escribir_inodo(inodo,ninodo);
        return 0;
    }

    if(inodo.tamEnBytesLog == 0 || offset > inodo.tamEnBytesLog){
        //fprintf(stderr,"Acceso fuera de rango.\n");
        return ACCESO_FUERA_DE_RANGO;
    }

    unsigned int
            firstByte = offset,
            lastByte = inodo.tamEnBytesLog < (offset + nbytes) ? inodo.tamEnBytesLog - 1 : (offset + nbytes) - 1,

            firstBloqueLogico = firstByte/BLOCKSIZE,
            lastBloqueLogico = lastByte/BLOCKSIZE,

            fByteRelativo = firstByte % BLOCKSIZE,
            lByteRelativo = lastByte % BLOCKSIZE,

            ptrFirstBloque = traducir_bloque_inodo(ninodo, firstBloqueLogico, 0),
            ptrLastBloque = traducir_bloque_inodo(ninodo, lastBloqueLogico, 0),
            bytesLeidos = 0;

    char buffer[BLOCKSIZE];
    switch (firstBloqueLogico == lastBloqueLogico){
        case 1:
            //fprintf(stderr,"Primer y unico bloque: En el bloque fisico %u desde la posicion del buffer_original %u, leemos desde el byte del buffer %u hasta la %u (%u bytes)\n",ptrFirstBloque, 0, fByteRelativo, lByteRelativo,lByteRelativo - fByteRelativo + 1);
            if(ptrFirstBloque != BLOQUE_LOGICO_NO_INICIALIZADO) {
                bread(ptrFirstBloque, buffer);
                memcpy(buf_original, buffer + fByteRelativo, lByteRelativo - fByteRelativo + 1);
            }
            bytesLeidos += lByteRelativo - fByteRelativo + 1;
            break;

        case 0:
            // Leemos del primero
            //fprintf(stderr,"Primer bloque (%u): En el bloque fisico %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u (%u bytes)\n",firstBloqueLogico, ptrFirstBloque, bytesLeidos, fByteRelativo, BLOCKSIZE-1,BLOCKSIZE - fByteRelativo);
            if(ptrFirstBloque != BLOQUE_LOGICO_NO_INICIALIZADO) {
                bread(ptrFirstBloque, buffer);
                memcpy(buf_original + bytesLeidos, buffer + fByteRelativo, BLOCKSIZE - fByteRelativo);
            }
            bytesLeidos += BLOCKSIZE - fByteRelativo;

            // Escribimos sobre los del medio
            for (int i = firstBloqueLogico+1; i <= lastBloqueLogico-1; ++i) {
                int ptrBloque = traducir_bloque_inodo(ninodo,i,0);
                //fprintf(stderr,"Bloque intermedio (%u): En el bloque %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u\n",i, ptrBloque, bytesLeidos, 0, BLOCKSIZE-1);
                if(ptrBloque != BLOQUE_LOGICO_NO_INICIALIZADO) {
                    bread(ptrBloque, buf_original + bytesLeidos);
                }
                bytesLeidos += BLOCKSIZE;
            }

            // Escribimos sobre el ultimo
            //fprintf(stderr,"Ultimo bloque (%u): En el bloque fisico %u desde la posicion del buffer_original %u, escribimos desde la posicion del buffer %u hasta la %u (%u bytes)\n",lastBloqueLogico, ptrLastBloque, bytesLeidos, 0, lByteRelativo,lByteRelativo+1);
            if(ptrLastBloque != BLOQUE_LOGICO_NO_INICIALIZADO) {
                bread(ptrLastBloque, buffer);
                memcpy(buf_original + bytesLeidos, buffer, lByteRelativo + 1);
            }
            bytesLeidos += lByteRelativo+1;
            break;

    }

    inodo.atime = time(NULL);

    escribir_inodo(inodo,ninodo);

    return bytesLeidos;
}

int mi_stat_f(unsigned int ninodo, stat_t *p_stat){
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    p_stat->tipo = inodo.tipo;
    p_stat->nlinks = inodo.nlinks;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->permisos = inodo.permisos;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;

    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    if(permisos > ((char) 7)){
        fprintf(stderr,"El valor %u no se corresponde a ninguna combinacion de permisos.\n",permisos);
        return PERMISOS_INVALIDOS;
    }

    inodo_t inodo;
    leer_inodo(ninodo, &inodo);
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    escribir_inodo(inodo, ninodo);

    return 0;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    if (!tiene_permiso(inodo.permisos, 'w')) {
        fprintf(stderr, "Este inodo no tiene permisos de escritura.\n");
        return PERMISOS_INSUFICIENTES;
    }

    if (inodo.tamEnBytesLog == 0 || nbytes > inodo.tamEnBytesLog-1) {
        fprintf(stderr, "El byte a partir del cual se quiere truncar (%u) es superior al ultimo byte escrito del inodo (%d).\n", nbytes, inodo.tamEnBytesLog-1);
        exit(ACCESO_FUERA_DE_RANGO);
    }

    unsigned int primerBlogico = nbytes % BLOCKSIZE ? (nbytes / BLOCKSIZE) + 1 : (nbytes / BLOCKSIZE);
    liberar_bloques_inodo(ninodo, primerBlogico);

    // Actualizamos times
    leer_inodo(ninodo, &inodo);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;

    escribir_inodo(inodo, ninodo);

    return 0;
}