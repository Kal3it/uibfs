#include "directorios/directorios.h"

int main(int argc, char const *argv[]){

    if (argc != 5) {
        puts("Uso: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <offset> <pathname>");
        return -1;
    }

    unsigned int offset = atoi(argv[3]);
    const char *buffer = argv[2];
    const char *camino = argv[4];
    const int len = strlen(buffer);

    bmount(argv[1]);

    int resultado = mi_write(camino,buffer,offset,len);
    if(resultado < 0) return resultado;

    printf("%d bytes escritos.\n",resultado);

    bumount();

    return 0;
}