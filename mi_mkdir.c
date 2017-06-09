#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_mkdir <nombre_dispositivo> <permisos> <path>");
        return -1;
    }


    char camino[strlen(argv[3])+2];
    strcpy(camino,argv[3]);
    char permisos = atoi(argv[2]);
    int resultado;

    bmount(argv[1]);

    if(camino[strlen(camino)-1] != '/') strcat(camino,"/");
    resultado = mi_creat(camino, permisos);

    bumount();

    return resultado;
}