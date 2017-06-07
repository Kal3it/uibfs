#include "directorios.h"

cache_t cache_read = { .num_entradas_cacheadas = 0, .entrada_mas_antigua = 0, .ultima_entrada_acertada = 0};
cache_t cache_write = { .num_entradas_cacheadas = 0, .entrada_mas_antigua = 0, .ultima_entrada_acertada = 0};

char extraer_camino(const char *camino, char *inicial, const char **final) {

    if(camino == NULL || camino[0] != '/' ){
        fprintf(stderr,"Pathname invalido.\n");
        return PATHNAME_INVALIDO;
    }

    unsigned int lenCamino = strlen(camino+1), lenFinal;

    if(lenCamino < 1 || lenCamino > MAX_TAM_NOMBRE_ENTRADA){
        fprintf(stderr,"Pathname invalido.\n");
        return PATHNAME_INVALIDO;
    }

    // Final apunta desde la primera barra si camino es un directorio o NULL si camino es un archivo
    *final = strchr(camino+1, '/');

    char tipo;
    if(*final == NULL){ // Si camino es un archivo
        tipo = 'f';
        *final = "/";
        lenFinal = 0;
    } else { // Si es un directorio
        tipo = 'd';
        lenFinal = strlen(*final);
    }

    strncpy(inicial, camino+1, lenCamino - lenFinal);
    inicial[lenCamino-lenFinal] = '\0';

    if(strlen(inicial) == 0){
        fprintf(stderr,"Pathname invalido.\n");
        return PATHNAME_INVALIDO;
    }

    //printf("inicial: %s, final: %s, lenCamino: %d, lenFinal: %d, size inicial: %d, size final: %d\n",inicial, *final, lenFinal, strlen(inicial),strlen(*final));

    return tipo;
}

void actualizar_cache(const char *camino, unsigned int ninodo, cache_t *cache){

    if (cache->num_entradas_cacheadas < TAM_CACHE) {
        entrada_cache_t nuevaEntrada;
        nuevaEntrada.ninodo = ninodo;
        strcpy(nuevaEntrada.nombre,camino);
        nuevaEntrada.ultimo_hit = time(NULL);

        cache->entradas[cache->num_entradas_cacheadas] = nuevaEntrada;
        ++cache->num_entradas_cacheadas;

    } else {
        cache->entradas[cache->entrada_mas_antigua].ninodo = ninodo;
        strcpy(cache->entradas[cache->entrada_mas_antigua].nombre,camino);
        cache->entradas[cache->entrada_mas_antigua].ultimo_hit = time(NULL);

    }
}

char buscar_entrada_en_cache(const char *camino, unsigned int *ninodo, cache_t *cache){
    entrada_cache_t entrada;
    int indice_entrada;
    char hit = 0;

    for (int i = 0; i < cache->num_entradas_cacheadas && !hit; ++i) {
        indice_entrada = (i+cache->ultima_entrada_acertada) % cache->num_entradas_cacheadas;

        entrada = cache->entradas[indice_entrada];
        hit = !strcmp(entrada.nombre, camino);

        if(cache->entradas[cache->entrada_mas_antigua].ultimo_hit > cache->entradas[indice_entrada].ultimo_hit){
            cache->entrada_mas_antigua = indice_entrada;
        }

        //fprintf(stderr,"Entrada encontrada en el indice %d. Entrada: %s, Ninodo: %d\n",indice_entrada,cache->entradas[indice_entrada].nombre,cache->entradas[indice_entrada].ninodo);
    }

    if(hit){
        cache->entradas[indice_entrada].ultimo_hit = time(NULL);
        cache->ultima_entrada_acertada = indice_entrada;
        *ninodo = cache->entradas[indice_entrada].ninodo;
    }

    return hit;
}

char buscar_entrada_directorio(unsigned int ninodo_dir, char *str_entrada, unsigned int *ninodo, unsigned int *ultima_entrada_leida){
    unsigned int
            max_entradas = 5,
            offset = 0,
            tamBuffer = max_entradas * sizeof(entrada_t),
            numEntrada = 0,
            entradas_leidas = 0;

    entrada_t entradas[max_entradas];
    int respuesta = 0;
    char existeEntrada = 0;

    //fprintf(stderr,"Buscando en el directorio con numero de inodo %u la entrada %s\n",ninodo_dir,str_entrada);

    respuesta = mi_read_f(ninodo_dir, entradas, offset, tamBuffer);
    *ultima_entrada_leida = -1;
    while (respuesta > 0){

        entradas_leidas = respuesta / sizeof(entrada_t);
        for(numEntrada = 0; numEntrada < entradas_leidas && !existeEntrada; ++numEntrada) {
            existeEntrada = !strcmp(entradas[numEntrada].nombre, str_entrada);
            ++(*ultima_entrada_leida);
            //fprintf(stderr,"Se lee la entrada %s (inodo %u) y se compara con la nuestra %s\n",entradas[numEntrada].nombre,entradas[numEntrada].ninodo, str_entrada);
        }
        if(existeEntrada) break;

        offset += (max_entradas * sizeof(entrada_t));
        respuesta = mi_read_f(ninodo_dir, entradas, offset, tamBuffer);
    }

    //fprintf(stderr,"Ultima entrada leida: %u\n",*ultima_entrada_leida);
    if(existeEntrada) *ninodo = entradas[numEntrada-1].ninodo;

    return existeEntrada;
}

void crear_entrada_directorio(unsigned int ninodo_dir, char *str_entrada, char tipo, char permisos, unsigned int *nuevo_ninodo){
    inodo_t inodo;
    leer_inodo(ninodo_dir, &inodo);

    entrada_t nueva_entrada;
    unsigned int offset = inodo.tamEnBytesLog;

    strcpy(nueva_entrada.nombre, str_entrada);
    nueva_entrada.ninodo = reservar_inodo(tipo, permisos);
    *nuevo_ninodo = nueva_entrada.ninodo;

    mi_write_f(ninodo_dir,&nueva_entrada,offset,sizeof(entrada_t));

    //fprintf(stderr,"Creado el ninodo %u con nombre '%s'.\n",nueva_entrada.ninodo,nueva_entrada.nombre);
}

/**
 *
 * @param ninodo_root
 * @param pathname
 * @param pninodo
 * @param reservar
 * @param permisos
 * @param pninodo_dir
 * @param pnentrada
 * @return 0 si OK, <0 si PERMISOS_INSUFICIENTES, NO_ES_DIRECTORIO, PATHNAME_INVALIDO, NO_EXISTE_ENTRADA, YA_EXISTE_ENTRADA
 */
int buscar_entrada(unsigned int ninodo_root,
                   const char pathname[],
                   unsigned int *pninodo,
                   char reservar,
                   unsigned char permisos,
                   unsigned int *pninodo_dir,
                   unsigned int *pnentrada){

    int esUltimoDir;
    unsigned int numEntradas, ultimaEntradaLeida, next_ninodo;
    char *dir, existeEntrada, tipo;
    const char *subdir;
    dir = malloc(MAX_TAM_NOMBRE_ENTRADA);

    while(strcmp(pathname,"/") != 0){

        inodo_t inodo_root;
        leer_inodo(ninodo_root, &inodo_root);

        numEntradas = inodo_root.tamEnBytesLog / sizeof(entrada_t);
        ultimaEntradaLeida = 0;
        next_ninodo = -1;
        existeEntrada = 0;
        tipo = -1;

        if((reservar && (inodo_root.permisos & 6) != 6)/* reservar & no rw */ || (!reservar && (inodo_root.permisos & 4) != 4) /* no reservar & no r */){
            fprintf(stderr,"Permisos insuficientes.\n");
            return PERMISOS_INSUFICIENTES;
        }
        if(inodo_root.tipo != TIPO_DIRECTORIO){
            fprintf(stderr,"No es un directorio!\n");
            return NO_ES_DIRECTORIO;
        }

        tipo = extraer_camino(pathname, dir, &subdir);
        if(((int) tipo) == PATHNAME_INVALIDO) return PATHNAME_INVALIDO;

        existeEntrada = buscar_entrada_directorio(ninodo_root, dir, &next_ninodo, &ultimaEntradaLeida);

        esUltimoDir = strcmp(subdir,"/") == 0;

        if(!existeEntrada){
            if(reservar && strcmp(subdir,"/") == 0){ // Si reservar y es la ultima entrada, creamos entrada
                crear_entrada_directorio(ninodo_root, dir, tipo, permisos, &next_ninodo);
            } else {
                fprintf(stderr,"El fichero o directorio '%s' no existe.\n",dir);
                return NO_EXISTE_ENTRADA;
            }
        } else if(reservar && esUltimoDir) {		// Entrada existe, hay que reservar y es el ultimo directorio de la ruta
            fprintf(stderr,"El fichero o directorio '%s' ya existe.\n",dir);
            return YA_EXISTE_ENTRADA;
        }

        //fprintf(stderr,"esUltimoDir %u, pninodo_dir %u, pnentrada %u\n",esUltimoDir, pninodo_dir != NULL ,pnentrada != NULL);
        if(esUltimoDir && (pninodo_dir != NULL )&& (pnentrada != NULL)){
            *pninodo_dir = ninodo_root;
            *pnentrada = existeEntrada ? ultimaEntradaLeida : numEntradas;
            //fprintf(stderr,"El ultimo directorio es %u y su entrada es %u\n",ninodo_root,*pnentrada);
        }

        ninodo_root = next_ninodo;
        pathname = subdir;
    }

    free(dir);
    *pninodo = ninodo_root;
    return 0;
}

int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem(SEM_DIRECTORIOS);

    unsigned int ninodo;

    int resultado = buscar_entrada(0, camino, &ninodo, 1, permisos, NULL, NULL);

    mi_signalSem(SEM_DIRECTORIOS);
    return resultado;
}

int mi_dir(const char *camino, char *buffer){
    unsigned int ninodo;
    inodo_t inodo;

    int resultado = buscar_entrada(0,camino,&ninodo,0,0, NULL, NULL);
    if(resultado < 0) return resultado;

    leer_inodo(ninodo,&inodo);

    // Comprobamos errores
    if((inodo.permisos & 2) != 2) /* r */{
        fprintf(stderr,"El fichero '%s' no tiene permisos de lectura.\n",camino);
        return PERMISOS_INSUFICIENTES;
    }
    if(inodo.tipo != TIPO_DIRECTORIO){
        fprintf(stderr,"%s no es un directorio!\n",camino);
        return NO_ES_DIRECTORIO;
    }

    // Creamos la cadena de salida
    inodo_t inodoEntrada;
    struct tm *tm;
    unsigned int
            numEntradas = inodo.tamEnBytesLog / sizeof(entrada_t),
            tamEntrada = 100;

    char
            *separador = " | ",
            strPermisos[5], strTamEnBytesLog[10], strTime[50], strTipo[2], strNinodo[10],
            strEntrada[tamEntrada * numEntradas];
    entrada_t bufferDirectorio[numEntradas];
    *strEntrada = '\0';

    resultado = mi_read_f(ninodo,bufferDirectorio,0,inodo.tamEnBytesLog);
    if(resultado < 0) return resultado;

    for (int i = 0; i < numEntradas; ++i) {
        leer_inodo(bufferDirectorio[i].ninodo, &inodoEntrada);

        // toStrings
        *strPermisos = '\0';
        *strNinodo = '\0';
        strTipo[0] = inodoEntrada.tipo;
        strTipo[1] = '\0';
        strcat(strPermisos, strTipo);
        strcat(strPermisos,(inodoEntrada.permisos & 4) ? "r" : "-");
        strcat(strPermisos,(inodoEntrada.permisos & 2) ? "w" : "-");
        strcat(strPermisos,(inodoEntrada.permisos & 1) ? "x" : "-");

        sprintf(strNinodo,"%u",bufferDirectorio[i].ninodo);
        sprintf(strTamEnBytesLog,"%u",inodoEntrada.tamEnBytesLog);

        tm = localtime(&inodoEntrada.mtime);
        sprintf(strTime,"[%d-%02d-%02d %02d:%02d:%02d]",tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

        strcat(strEntrada,strNinodo);
        strcat(strEntrada,separador);
        strcat(strEntrada,strPermisos);
        strcat(strEntrada,separador);
        strcat(strEntrada,strTamEnBytesLog);
        strcat(strEntrada,separador);
        strcat(strEntrada,strTime);
        strcat(strEntrada,separador);
        strcat(strEntrada,bufferDirectorio[i].nombre);
        strcat(strEntrada,"\n");
    }

    strcpy(buffer,strEntrada);

    return 0;
}

int mi_dir_simple(const char *camino, void *buffer_entradas){
    unsigned int ninodo;
    inodo_t inodo;

    int resultado = buscar_entrada(0,camino,&ninodo,0,0, NULL, NULL);
    if(resultado < 0) return resultado;
    leer_inodo(ninodo,&inodo);

    if(inodo.tipo != TIPO_DIRECTORIO){
        fprintf(stderr,"%s no es un directorio!\n",camino);
        return NO_ES_DIRECTORIO;
    }

    resultado = mi_read_f(ninodo,buffer_entradas,0,inodo.tamEnBytesLog);
    if(resultado < 0) return resultado;

    return (resultado / sizeof(entrada_t));
}

int mi_link(const char *camino1, const char *camino2){
    mi_waitSem(SEM_DIRECTORIOS);

    inodo_t inodo_fichero, inodo_dir;
    int resultado;
    unsigned int ninodo_fichero, ninodo_enlace, ninodo_dir, nentrada;

    resultado = buscar_entrada(0, camino1, &ninodo_fichero, 0, 0, NULL, NULL);
    if(resultado < 0){
        mi_signalSem(SEM_DIRECTORIOS);
        return resultado;
    }

    // Comprobamos errores
    leer_inodo(ninodo_fichero,&inodo_fichero);
    if((inodo_fichero.permisos & 2) != 2) /* r */{
        mi_signalSem(SEM_DIRECTORIOS);
        fprintf(stderr,"El fichero '%s' no tiene permisos de lectura.\n",camino1);
        return PERMISOS_INSUFICIENTES;
    }
    if(inodo_fichero.tipo != TIPO_FICHERO){
        mi_signalSem(SEM_DIRECTORIOS);
        fprintf(stderr,"%s no es un fichero!\n",camino1);
        return NO_ES_FICHERO;
    }

    resultado = buscar_entrada(0, camino2, &ninodo_enlace, 1, 7, &ninodo_dir, &nentrada);
    if(resultado < 0){
        mi_signalSem(SEM_DIRECTORIOS);
        return resultado;
    }

    liberar_inodo(ninodo_enlace);

    // Actualizamos entrada
    leer_inodo(ninodo_dir,&inodo_dir);
    entrada_t bufferEntradas[inodo_dir.tamEnBytesLog / sizeof(entrada_t)];

    resultado = mi_read_f(ninodo_dir, bufferEntradas, 0, inodo_dir.tamEnBytesLog);
    if(resultado < 0){
        mi_signalSem(SEM_DIRECTORIOS);
        return resultado;
    }

    bufferEntradas[nentrada].ninodo = ninodo_fichero;
    //fprintf(stderr,"bufferEntradas[%u].ninodo = %u\n",nentrada,bufferEntradas[nentrada].ninodo);
    mi_write_f(ninodo_dir, bufferEntradas, 0, inodo_dir.tamEnBytesLog);

    // Actualizamos info inodo_fichero
    //fprintf(stderr,"Actualizamos datos del ninodo %u\n", ninodo_fichero);
    leer_inodo(ninodo_fichero, &inodo_fichero);
    ++inodo_fichero.nlinks;
    inodo_fichero.ctime = time(NULL);
    escribir_inodo(inodo_fichero, ninodo_fichero);

    mi_signalSem(SEM_DIRECTORIOS);
    return 0;
}

int mi_unlink(const char *camino){
    mi_waitSem(SEM_DIRECTORIOS);

    inodo_t inodo, inodo_dir;
    int resultado;
    unsigned int ninodo = -1, ninodo_dir = -1, nentrada = -1, nentradas_inodo_dir = -1;

    resultado = buscar_entrada(0, camino, &ninodo, 0, 0, &ninodo_dir, &nentrada);
    if(resultado < 0){
        mi_signalSem(SEM_DIRECTORIOS);
        return resultado;
    }

    leer_inodo(ninodo, &inodo);
    leer_inodo(ninodo_dir, &inodo_dir);

    if(inodo.tipo == TIPO_DIRECTORIO && inodo.tamEnBytesLog > 0){
        mi_signalSem(SEM_DIRECTORIOS);
        fprintf(stderr,"No es posible borrar la entrada '%u' porque es un directorio no vacio.\n",nentrada);
        return IMPOSIBLE_BORRAR_ENTRADA;
    }

    nentradas_inodo_dir = inodo_dir.tamEnBytesLog / sizeof(entrada_t);

    int esUltimaEntrada = (nentrada == nentradas_inodo_dir-1);
    if (!esUltimaEntrada)
    {
        entrada_t bufferEntradas[nentradas_inodo_dir];

        resultado = mi_read_f(ninodo_dir,bufferEntradas,0,inodo_dir.tamEnBytesLog);
        if(resultado < 0){
            mi_signalSem(SEM_DIRECTORIOS);
            return resultado;
        }

        bufferEntradas[nentrada] = bufferEntradas[nentradas_inodo_dir-1];

        resultado = mi_write_f(ninodo_dir,bufferEntradas,0,inodo_dir.tamEnBytesLog);
        if(resultado < 0){
            mi_signalSem(SEM_DIRECTORIOS);
            return resultado;
        }
    }

    mi_truncar_f(ninodo_dir,(nentradas_inodo_dir-1)*sizeof(struct entrada));

    --inodo.nlinks;
    if (inodo.nlinks == 0) {
        liberar_inodo(ninodo);
    }
    else {
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        escribir_inodo(inodo, ninodo);
    }

    mi_signalSem(SEM_DIRECTORIOS);

    return 0;
}

int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int ninodo;

    int resultado = buscar_entrada(0,camino,&ninodo,0,0,NULL,NULL);
    if(resultado < 0) return resultado;

    resultado = mi_chmod_f(ninodo,permisos);
    if(resultado < 0) return resultado;

    return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int ninodo;
    
    int resultado = buscar_entrada(0,camino,&ninodo,0,0,NULL,NULL);
    if(resultado < 0) return resultado;

    resultado = mi_stat_f(ninodo,p_stat);
    if(resultado < 0) return resultado;

    return 0;
}
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    unsigned int ninodo;
    int resultado;

    char hit = buscar_entrada_en_cache(camino, &ninodo, &cache_read);
    if(!hit){
        resultado = buscar_entrada(0, camino, &ninodo, 0, 0, NULL, NULL);
        if(resultado < 0) return resultado;
        actualizar_cache(camino, ninodo, &cache_read);
    }

    leer_inodo(ninodo,&inodo);
    if(inodo.tipo != TIPO_FICHERO){
        fprintf(stderr,"%s no es un fichero.\n",camino);
        return NO_ES_FICHERO;
    }

    resultado = mi_read_f(ninodo,buf,offset,nbytes);

    return resultado;
}
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    mi_waitSem(SEM_DIRECTORIOS);// todo los problemas vienen de poner este semaforo abajo
    unsigned int ninodo;
    inodo_t inodo;
    int resultado;

    char hit = buscar_entrada_en_cache(camino,&ninodo, &cache_write);
    if(!hit){
        resultado = buscar_entrada(0, camino, &ninodo, 0, 0, NULL, NULL);
        if(resultado < 0) return resultado;
        actualizar_cache(camino, ninodo, &cache_write);
    }

//    mi_waitSem(SEM_DIRECTORIOS);
    leer_inodo(ninodo,&inodo);
    if(inodo.tipo != TIPO_FICHERO){
        fprintf(stderr,"%s no es un fichero.\n",camino);
        return NO_ES_FICHERO;
    }


    resultado = mi_write_f(ninodo,buf,offset,nbytes);
    mi_signalSem(SEM_DIRECTORIOS);

    return resultado;
}
