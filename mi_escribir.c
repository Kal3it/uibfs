#include "directorios/directorios.h"

int main(int argc, char const *argv[]){

    if (argc != 5) {
        puts("Uso: mi_escribir <nombre_dispositivo> <pathname> <\"$(cat fichero)\"> <offset> ");
        return -1;
    }

    unsigned int offset = atoi(argv[4]);
    const char *buffer = argv[3];
    const char *camino = argv[2];
    const int len = strlen(buffer);

    bmount(argv[1]);

    int resultado = mi_write(camino,buffer,offset,len);
    if(resultado < 0) return resultado;

    printf("%d bytes escritos.\n",resultado);

    bumount();

    return 0;
}