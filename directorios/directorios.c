#include "directorios.h"

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

    if(strcmp(pathname,"/") == 0){
        *pninodo = ninodo_root;
        return 0;

    } else {

        inodo_t inodo_root;
        leer_inodo(ninodo_root, &inodo_root);

        int esUltimoDir;
        unsigned int
                numEntradas = inodo_root.tamEnBytesLog / sizeof(entrada_t),
                ultimaEntradaLeida = 0,
                next_ninodo = -1;
        char
                dir[MAX_TAM_NOMBRE_ENTRADA],
                existeEntrada = 0,
                tipo = -1;
        const int TAM_BUFFER = numEntradas + 1; // +1 entrada por si hay que crearla
        const char *subdir;

        entrada_t buffer[TAM_BUFFER];

        if(!((reservar && (inodo_root.permisos & 3))/* reservar & rw */ || (!reservar && (inodo_root.permisos & 2)) /* no reservar & r */)){
            fprintf(stderr,"Permisos insuficientes.\n");
            return PERMISOS_INSUFICIENTES;
        }
        if(inodo_root.tipo != TIPO_DIRECTORIO){
            fprintf(stderr,"No es un directorio!\n");
            return NO_ES_DIRECTORIO;
        }

        tipo = extraer_camino(pathname, dir, &subdir);
        if(((int) tipo) == PATHNAME_INVALIDO) return PATHNAME_INVALIDO;

//        fprintf(stderr,"Buscando en el directorio con numero de inodo %u la entrada %s\n",ninodo_root,dir);

        /* Buscar entrada dir en inodo_root y recuperar el numero de entrada y el ninodo */
        mi_read_f(ninodo_root, buffer, 0, inodo_root.tamEnBytesLog); //todo Cuidado si el directorio tiene gigas en entradas...
        while(ultimaEntradaLeida < numEntradas && !existeEntrada) {
            existeEntrada = !strcmp(buffer[ultimaEntradaLeida].nombre, dir);
//            fprintf(stderr,"Se lee la entrada del directorio %s y se compara con la nuestra %s\n",buffer[ultimaEntradaLeida].nombre,dir);

            if(existeEntrada){
                next_ninodo = buffer[ultimaEntradaLeida].ninodo;
            }

            ++ultimaEntradaLeida;
        }
        --ultimaEntradaLeida;

        esUltimoDir = strcmp(subdir,"/") == 0;

        if(!existeEntrada){
            if(reservar && strcmp(subdir,"/") == 0){ // Si reservar y es la ultima entrada, creamos entrada
                buffer[numEntradas].ninodo = reservar_inodo(tipo,permisos); // se crea en la posicion adicional
                strcpy(buffer[numEntradas].nombre,dir);
                mi_write_f(ninodo_root,buffer,0,TAM_BUFFER * sizeof(entrada_t));
                next_ninodo = buffer[numEntradas].ninodo;
                printf("Creado el ninodo %u en la entrada %u con nombre '%s'.\n",buffer[numEntradas].ninodo,numEntradas,buffer[numEntradas].nombre);
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
            fprintf(stderr,"El ultimo directorio es %u y su entrada es %u\n",ninodo_root,*pnentrada);
        }

        int respuesta = buscar_entrada(next_ninodo, subdir, pninodo, reservar, permisos, pninodo_dir, pnentrada);

//        // Revertir cambios en caso de estar en modo escritura y haya fallado
//        if(respuesta == YA_EXISTE_ENTRADA || respuesta == PATHNAME_INVALIDO || respuesta == NO_ES_DIRECTORIO){
//            if(reservar && !existeEntrada){// Caso en el que se ha escrito
//                mi_truncar_f(ninodo_root, numEntradas); // Se elimina la entrada adicional
//            }
//        }

        return respuesta;
    }
}

int mi_creat(const char *camino, unsigned char permisos){
    unsigned int ninodo;
    struct superbloque sb;
    bread(posSB,&sb);

    return buscar_entrada(sb.posInodoRaiz, camino, &ninodo, 1, permisos, NULL, NULL);
}

int mi_dir(const char *camino, char *buffer){
    unsigned int ninodo;
    inodo_t inodo;
    struct superbloque sb;
    bread(posSB,&sb);

    int resultado = buscar_entrada(sb.posInodoRaiz,camino,&ninodo,0,0, NULL, NULL);

    if(resultado < 0){
        return resultado;
    }

    leer_inodo(ninodo,&inodo);

    // Comprobamos errores
    if(!(inodo.permisos & 2)) /* r */{
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

    mi_read_f(ninodo,bufferDirectorio,0,inodo.tamEnBytesLog); //todo cuidado si tiene gigas

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

    fprintf(stderr,"%s",strEntrada);

    return 0;
}


int mi_link(const char *camino1, const char *camino2){
    inodo_t inodo_fichero, inodo_dir;
    int resultado;
    unsigned int ninodo_fichero, ninodo_enlace, ninodo_dir, nentrada;
    struct superbloque sb;
    bread(posSB,&sb);

    resultado = buscar_entrada(sb.posInodoRaiz, camino1, &ninodo_fichero, 0, 0, NULL, NULL);
    if(resultado < 0) return resultado;

    // Comprobamos errores
    leer_inodo(ninodo_fichero,&inodo_fichero);
    if(!(inodo_fichero.permisos & 2)) /* r */{
        fprintf(stderr,"El fichero '%s' no tiene permisos de lectura.\n",camino1);
        return PERMISOS_INSUFICIENTES;
    }
    if(inodo_fichero.tipo != TIPO_FICHERO){
        fprintf(stderr,"%s no es un fichero!\n",camino1);
        return NO_ES_FICHERO;
    }

    resultado = buscar_entrada(sb.posInodoRaiz, camino2, &ninodo_enlace, 1, 7, &ninodo_dir, &nentrada);
    if(resultado < 0) return resultado;

    liberar_inodo(ninodo_enlace);

    // Actualizamos entrada
    leer_inodo(ninodo_dir,&inodo_dir);
    entrada_t bufferEntradas[(inodo_dir.tamEnBytesLog / sizeof(entrada_t))];
    fprintf(stderr,"El buffer de entradas tiene %u entradas.\n",(inodo_dir.tamEnBytesLog / sizeof(entrada_t)));

    mi_read_f(ninodo_dir, bufferEntradas, 0, inodo_dir.tamEnBytesLog); //todo: cuidado tamaño
    bufferEntradas[nentrada].ninodo = ninodo_fichero;
    fprintf(stderr,"bufferEntradas[%u].ninodo = %u\n",nentrada,bufferEntradas[nentrada].ninodo);
    mi_write_f(ninodo_dir, bufferEntradas, 0, inodo_dir.tamEnBytesLog);//todo: quiza se pueda optimizar con el offset

    // Actualizamos info inodo_fichero
    fprintf(stderr,"Actualizamos datos del ninodo %u\n", ninodo_fichero);
    leer_inodo(ninodo_fichero, &inodo_fichero);
    ++inodo_fichero.nlinks;
    inodo_fichero.ctime = time(NULL);
    escribir_inodo(inodo_fichero, ninodo_fichero);

    return 0;
}

int mi_unlink(const char *camino){

}