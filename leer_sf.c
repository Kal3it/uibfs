#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "bloques/bloques.h"
#include "ficheros_basico/ficheros_basico.h"

void imprimirInodoInfo(unsigned int ninodo){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);

    printf("INFORMACION DEL INODO: \n");
    printf("Ninodo: %u\n",ninodo);
    printf("Tipo: %c\n",inodo.tipo);
    printf("Permisos: %u\n",inodo.permisos);
    printf("atime: %s\n",ctime(&inodo.atime));
    printf("mtime: %s\n",ctime(&inodo.mtime));
    printf("ctime: %s\n",ctime(&inodo.ctime));
    printf("nlinks: %u\n",inodo.nlinks);
    printf("tamEnBytesLog: %u\n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",inodo.numBloquesOcupados);

    printf("punterosDirectos: %u\n",inodo.punterosDirectos[0]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[1]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[2]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[3]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[4]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[5]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[6]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[7]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[8]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[9]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[10]);
    printf("punterosDirectos: %u\n",inodo.punterosDirectos[11]);
    printf("punterosIndirectos0: %u\n",inodo.punterosIndirectos[0]);
    printf("punterosIndirectos1: %u\n",inodo.punterosIndirectos[1]);
    printf("punterosIndirectos2: %u\n",inodo.punterosIndirectos[2]);
    printf("\n");
}

void imprimirSbInfo(){
    struct superbloque sb;
    bread(posSB,&sb);

    puts("INFORMACION DEL SUPERBLOQUE");
    printf("Primer bloque MB: %d\n",sb.posPrimerBloqueMB);
    printf("Ultimo bloque MB: %d\n",sb.posUltimoBloqueMB);
    printf("Primer bloque AI: %d\n",sb.posPrimerBloqueAI);
    printf("Ultimo bloque AI: %d\n",sb.posUltimoBloqueAI);
    printf("Primer bloque Datos: %d\n",sb.posPrimerBloqueDatos);
    printf("Ultimo bloque Datos: %d\n",sb.posUltimoBloqueDatos);
    printf("Posicion del inodo raiz: %d\n",sb.posInodoRaiz);
    printf("Posicion del primer inodo libre: %d\n",sb.posPrimerInodoLibre);
    printf("Cantidad de bloques libres: %d\n",sb.cantBloquesLibres);
    printf("Cantidad de inodos libres: %d\n",sb.cantInodosLibres);
    printf("Bloques totales: %d\n",sb.totBloques);
    printf("Inodos totales: %d\n",sb.totInodos);
    printf("Sizeof superbloque: %lu\n", sizeof(struct superbloque));
    printf("Sizeof inodo: %lu\n", sizeof(inodo_t));
    puts("");
}

int main(int argc, char const *argv[]) {

    if (argc != 2) {
        puts("Uso: leer_sf <nombre_dispositivo>");
        return -1;
    }

    bmount(argv[1]);

    imprimirSbInfo();

    struct superbloque sb;
    bread(posSB,&sb);

    for (int i = 0; i < sb.posPrimerInodoLibre; ++i) {
        imprimirInodoInfo(i);
    }

    bumount();

    return 0;
}