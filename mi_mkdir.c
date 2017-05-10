#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_creat <nombre_dispositivo> <permisos> <path>");
        return -1;
    }


    char camino[strlen(argv[3])+1];
    strcpy(camino,argv[3]);
    char permisos = atoi(argv[2]);
    int resultado;

    bmount(argv[1]);

    if(camino[strlen(camino)-1] != '/') strcat(camino,"/");
    fprintf(stderr,"%s\n",camino);
    resultado = mi_creat(camino, permisos);

    bumount();

    return resultado;
}