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
	    write(1,buffer,respuesta);
        bytesLeidos += respuesta;
        offset += tamBuffer;
        memset(buffer,0,tamBuffer);
        respuesta = mi_read(camino, buffer, offset, tamBuffer);
    }

    char infobuff[100];
    sprintf(infobuff,"%d bytes leidos\n",bytesLeidos);
    write(2,infobuff, strlen(infobuff));
    bumount();

    return 0;
}