#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "etapa1/bloques.h"
#include "etapa2/ficheros_basico.h"

int main(int argc, char const *argv[])
{
    if(argc != 2){
        puts("Uso: leer_sf <nombre_dispositivo>");
        return -1;
    }

    struct superbloque * sb = malloc(sizeof(struct superbloque));

    bmount(argv[1]);

    // Leer superbloque
    bread(posSB,sb);

    printf("Primer bloque MB: %d\n",sb->posPrimerBloqueMB);
    printf("Ultimo bloque MB: %d\n",sb->posUltimoBloqueMB);
    printf("Primer bloque AI: %d\n",sb->posPrimerBloqueAI);
    printf("Ultimo bloque AI: %d\n",sb->posUltimoBloqueAI);
    printf("Primer bloque Datos: %d\n",sb->posPrimerBloqueDatos);
    printf("Ultimo bloque Datos: %d\n",sb->posUltimoBloqueDatos);
    printf("Posicion del inodo raiz: %d\n",sb->posInodoRaiz);
    printf("Posicion del primer inodo libre: %d\n",sb->posPrimerInodoLibre);
    printf("Cantidad de bloques libres: %d\n",sb->cantBloquesLibres);
    printf("Cantidad de inodos libres: %d\n",sb->cantInodosLibres);
    printf("Bloques totales: %d\n",sb->totBloques);
    printf("Inodos totales: %d\n",sb->totInodos);

    bumount();
}