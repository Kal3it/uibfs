#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        puts("Uso: mi_rm <nombre_dispositivo> <pathname>");
        return -1;
    }

    const char *camino = argv[2];
    int resultado;

    bmount(argv[1]);

    resultado = mi_unlink(camino);

    bumount();

    return resultado;
}