#include "directorios/directorios.h"

int main(int argc, char const *argv[]){

    if (argc != 4) {
        puts("Uso: mi_mv <nombre_dispositivo> <pathname original> <pathname nuevo>");
        return -1;
    }

    const char *camino1 = argv[2], *camino2 = argv[3];
    int resultado;

    bmount(argv[1]);

    resultado = mi_link(camino1, camino2);
    if(resultado < 0) return resultado;
    resultado = mi_unlink(camino1);
    if(resultado < 0) return resultado;

    bumount();

    return 0;
}