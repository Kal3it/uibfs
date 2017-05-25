#include "directorios/directorios.h"

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: mi_touch <nombre_dispositivo> <permisos> <path>");
        return -1;
    }


    char camino[strlen(argv[3])+1];
    strcpy(camino,argv[3]);
    char permisos = atoi(argv[2]);
    int resultado;

    bmount(argv[1]);

    if(camino[strlen(camino)-1] == '/'){
        fprintf(stderr,"No es posible crear directorios con %s.\n",argv[0]);
        return PATHNAME_INVALIDO;
    }
    resultado = mi_creat(camino, permisos);

    bumount();

    return resultado;
}