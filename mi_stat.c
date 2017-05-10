#include "directorios/directorios.h"

void imprimirStat(stat_t stat){
    printf("Tipo: %c\n",stat.tipo);
    printf("Permisos: %u\n",stat.permisos);
    printf("atime: %s\n",ctime(&stat.atime));
    printf("mtime: %s\n",ctime(&stat.mtime));
    printf("ctime: %s\n",ctime(&stat.ctime));
    printf("nlinks: %u\n",stat.nlinks);
    printf("tamEnBytesLog: %u\n",stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",stat.numBloquesOcupados);
    printf("\n");
}

int main(int argc, char const *argv[]){
    if (argc != 3) {
        puts("Uso: mi_creat <nombre_dispositivo> <path>");
        return -1;
    }


    const char *camino = argv[2];
    int resultado;

    bmount(argv[1]);

    stat_t stat;
    resultado = mi_stat(camino, &stat);
    if(resultado < 0) return resultado;
    imprimirStat(stat);

    bumount();

    return resultado;
}