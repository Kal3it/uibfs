#include "ficheros/ficheros.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        puts("Uso: leer <nombre_dispositivo> <ninodo>");
        return -1;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int tamBuffer = BLOCKSIZE;
    unsigned int offset = 0, bytesLeidos = 0;
    char buffer[tamBuffer];

    bmount(argv[1]);

    memset(buffer,0,tamBuffer);
    int respuesta = mi_read_f(ninodo, buffer, offset, tamBuffer);
    while(respuesta != ACCESO_FUERA_DE_RANGO && respuesta != PERMISOS_INSUFICIENTES){
        for (int i = 0; i < respuesta; ++i) {
            printf("%c",buffer[i]);
        }
        bytesLeidos += respuesta;
        offset += tamBuffer;
        memset(buffer,0,tamBuffer);
        respuesta = mi_read_f(ninodo, buffer, offset, tamBuffer);
    }

    fprintf(stderr,"Bytes leidos totales: %u\n",bytesLeidos);
    stat_t stat;
    mi_stat_f(ninodo,&stat);
    fprintf(stderr,"tamEnBytesLog: %u\n",stat.tamEnBytesLog);

    bumount();

    return 0;
}
