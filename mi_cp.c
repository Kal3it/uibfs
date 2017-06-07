#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_cp <nombre_dispositivo> <pathname original> <pathname nuevo>");
        return -1;
    }

    const char *camino1 = argv[2], *camino2 = argv[3];
    unsigned int tamBuffer = BLOCKSIZE;
    unsigned int offset = 0, bytesCopiados = 0;
    char buffer[tamBuffer];
    char zerotest[tamBuffer];
    memset(zerotest,0,tamBuffer);
    int respuesta;

    bmount(argv[1]);

    if(camino2[strlen(camino2) - 1] == '/'){
        fprintf(stderr,"%s es un directorio.\n",camino2);
        return NO_ES_FICHERO;
    }

    stat_t stat_camino1;
    respuesta = mi_stat(camino1,&stat_camino1);
    if(respuesta < 0) return respuesta;

    if(stat_camino1.tipo != TIPO_FICHERO){
        fprintf(stderr,"%s no es un fichero.\n",camino1);
        return NO_ES_FICHERO;
    }

    mi_creat(camino2,stat_camino1.permisos);
    if(respuesta < 0) return respuesta;

    const char CENTINELA = 1;
    buffer[tamBuffer] = CENTINELA;
    memset(buffer,0,tamBuffer);
    respuesta = mi_read(camino1, buffer, offset, tamBuffer);
    while(respuesta > 0){

        // Comprobamos si el bloque contiene datos
        if (memcmp(buffer,zerotest,tamBuffer)){
            mi_write(camino2,buffer,offset,respuesta);
            memset(buffer,0,tamBuffer);
        }
        //else printf("Bloque logico %u no inicializado, skipping\n",offset/tamBuffer);

        bytesCopiados += respuesta;
        offset += tamBuffer;
        respuesta = mi_read(camino1, buffer, offset, tamBuffer);
    }

    fprintf(stderr,"Bytes copiados: %u\n",bytesCopiados);

    bumount();

    return 0;
}
