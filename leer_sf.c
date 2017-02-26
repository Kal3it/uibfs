#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "etapa1/bloques.h"
#include "etapa2/ficheros_basico.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        puts("Uso: leer_sf <nombre_dispositivo>");
        return -1;
    }

    struct superbloque *sb = malloc(sizeof(struct superbloque));

    bmount(argv[1]);

    // Leer superbloque
    if(bread(posSB, sb) == -1) return -1;

    puts("Informacion del Superbloque:");
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
    printf("Sizeof superbloque: %lu\n", sizeof(struct superbloque));
    printf("Sizeof inodo: %lu\n", sizeof(inodo_t));

    // Recorrido de los inodos libres
    inodo_t inodos[BLOCKSIZE / T_INODO];

    unsigned int
            nextBloque = sb->posPrimerBloqueAI,
            currBloque,
            nextInodo = 0;

    if(bread(nextBloque, &inodos) == -1) return -1;

    while (inodos[nextInodo].punterosDirectos[0] != UINT_MAX) {
        printf("El inodo %u del bloque %u apunta al inodo %u\n", nextInodo, nextBloque,inodos[nextInodo].punterosDirectos[0]);

        currBloque = nextBloque;
        nextBloque = (sb->posPrimerBloqueAI) + (inodos[nextInodo].punterosDirectos[0] / (BLOCKSIZE / T_INODO));
        nextInodo = inodos[nextInodo].punterosDirectos[0] % (BLOCKSIZE / T_INODO);
        if(currBloque < nextBloque) if(bread(nextBloque, &inodos) == -1) return -1; // Solo leemos el bloque si es diferente al anterior, para optimizar
    }

    printf("El inodo %u del bloque %u no apunta a nada (valor UINT_MAX = %u)\n", nextInodo, nextBloque,inodos[nextInodo].punterosDirectos[0]);

//    puts("");
//    printf("Informacion del inodo %d\n",0);
//    printf("atime del inodo %d: %lu\n",0,inodos[0].atime);
//    printf("mtime del inodo %d: %lu\n",0,inodos[0].mtime);
//    printf("ctime del inodo %d: %lu\n",0,inodos[0].ctime);
//    printf("tipo del inodo %d: %c\n",0,inodos[0].tipo);
//    printf("permisos del inodo %d: %o\n",0,inodos[0].permisos);
//    printf("nlinks del inodo %d: %u\n",0,inodos[0].nlinks);
//    printf("tamEnBytesLog del inodo %u: %o\n",0,inodos[0].tamEnBytesLog);
//    printf("Numero de bloques ocupados del inodo %d: %u\n",0,inodos[0].tamEnBytesLog);
//    printf("Puntero directo %d del inodo %d: %u\n",0,0,inodos[0].punterosDirectos[0]);

    if(bumount() == -1) return -1;

    return 0;
}
