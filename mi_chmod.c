#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_creat <nombre_dispositivo> <permisos> <path>");
        return -1;
    }

    const char *camino = argv[3];
    char permisos = atoi(argv[2]);
    int resultado;

    bmount(argv[1]);

    resultado = mi_chmod(camino,permisos);

    bumount();

    return resultado;
}