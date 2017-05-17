#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        puts("Uso: mi_leer <nombre_dispositivo> <pathname>");
        return -1;
    }

    const char *camino = argv[2];
    unsigned int tamBuffer = BLOCKSIZE;
    unsigned int offset = 0, bytesLeidos = 0;
    char buffer[tamBuffer];

    bmount(argv[1]);

    memset(buffer,0,tamBuffer);
    int respuesta = mi_read(camino, buffer, offset, tamBuffer);
    while(respuesta > 0){
        for (int i = 0; i < respuesta; ++i) {
            printf("%c",buffer[i]);
        }
        bytesLeidos += respuesta;
        offset += tamBuffer;
        memset(buffer,0,tamBuffer);
        respuesta = mi_read(camino, buffer, offset, tamBuffer);
    }

    fprintf(stderr,"Bytes leidos totales: %u\n",bytesLeidos);

    bumount();

    return 0;
}