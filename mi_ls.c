#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 3) {
        puts("Uso: mi_creat <nombre_dispositivo> <path>");
        return -1;
    }

    const char *camino = argv[2];
    char buffer[1000];
    int resultado;

    bmount(argv[1]);

    resultado = mi_dir(camino, buffer);
    if(resultado < 0) return resultado;

    printf("%s",buffer);

    bumount();

    return resultado;
}