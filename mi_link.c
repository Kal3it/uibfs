#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_link <nombre_dispositivo> <pathname src> <pathname dst>");
        return -1;
    }

    const char *camino1 = argv[2], *camino2 = argv[3];
    int resultado;

    bmount(argv[1]);

    resultado = mi_link(camino1,camino2);

    bumount();

    return resultado;
}