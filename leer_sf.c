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

    struct superbloque sb;

    bmount(argv[1]);

    // Leer superbloque
    if(bread(posSB, &sb) == -1) return -1;

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

    // Recorrido de los inodos libres
//    inodo_t inodos[BLOCKSIZE / T_INODO];
//
//    unsigned int
//            nextBloque = sb.posPrimerBloqueAI,
//            currBloque,
//            nextInodo = 0;
//
//    if(bread(nextBloque, &inodos) == -1) return -1;

//    while (inodos[nextInodo].punterosDirectos[0] != UINT_MAX) {
//        printf("El inodo %u del bloque %u apunta al inodo %u\n", nextInodo, nextBloque,inodos[nextInodo].punterosDirectos[0]);
//
//        currBloque = nextBloque;
//        nextBloque = (sb.posPrimerBloqueAI) + (inodos[nextInodo].punterosDirectos[0] / (BLOCKSIZE / T_INODO));
//        nextInodo = inodos[nextInodo].punterosDirectos[0] % (BLOCKSIZE / T_INODO);
//        if(currBloque < nextBloque) if(bread(nextBloque, &inodos) == -1) return -1; // Solo leemos el bloque si es diferente al anterior, para optimizar
//    }
//
//    printf("El inodo %u del bloque %u no apunta a nada (valor UINT_MAX = %u)\n", nextInodo, nextBloque,inodos[nextInodo].punterosDirectos[0]);

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

    inodo_t inodoRaiz;
    if(leer_inodo(sb.posInodoRaiz,&inodoRaiz) == -1){
        return -1;
    }

	puts("DATOS DEL DIRECTORIO RAIZ");
	printf("Tipo: %c\n",inodoRaiz.tipo);
	printf("Permisos: %u\n",inodoRaiz.permisos);
	//printf("atime: %d\n",inodoRaiz.atime); // todo: imprimirlo en el formato correspondiente
    //printf("mtime: %d\n",inodoRaiz.mtime);
    //printf("ctime: %d\n",inodoRaiz.ctime);
	printf("nlinks: %u\n",inodoRaiz.nlinks);
	printf("tamEnBytesLog: %u\n",inodoRaiz.tamEnBytesLog);
	printf("numBloquesOcupados: %u\n",inodoRaiz.numBloquesOcupados);
    puts("");
	
	puts("MAPA DE BITS");
	printf("valorSB: %u\n",leer_bit(posSB));
	printf("valorPrimerBloqueMB: %u\n",leer_bit(sb.posPrimerBloqueMB));
	printf("valorUltimoBloqueMB: %u\n",leer_bit(sb.posUltimoBloqueMB));
	printf("valorPrimerBloqueAI: %u\n",leer_bit(sb.posPrimerBloqueAI));
	printf("valorUltimoBloqueAI: %u\n",leer_bit(sb.posUltimoBloqueAI));
	printf("valorPrimerBloqueDatos: %u\n",leer_bit(sb.posPrimerBloqueDatos));
	printf("valorUltimoBloqueDatos: %u\n",leer_bit(sb.posUltimoBloqueDatos));
    puts("");

//    inodo_t inodo;
//    unsigned int bufferBloqueIndice[NPUNTEROS];
//    unsigned int ninodo = reservar_inodo('f',7);
//    unsigned int bloque0, bloque1, bloque2, bloque3;
//
//    leer_inodo(ninodo, &inodo);
//    inodo.punterosIndirectos[2] = reservar_bloque();
//    bloque0 = inodo.punterosIndirectos[2];
//
//    bread(inodo.punterosIndirectos[2],bufferBloqueIndice);
//    bufferBloqueIndice[5] = reservar_bloque();
//    bwrite(inodo.punterosIndirectos[2],bufferBloqueIndice);
//    bloque1 = bufferBloqueIndice[5];
//
//    unsigned int ptr = bufferBloqueIndice[5];
//    bread(ptr,bufferBloqueIndice);
//    bufferBloqueIndice[25] = reservar_bloque();
//    bwrite(ptr,bufferBloqueIndice);
//    bloque2 = bufferBloqueIndice[25];
//
//    ptr = bufferBloqueIndice[25];
//    bread(ptr,bufferBloqueIndice);
//    bufferBloqueIndice[120] = reservar_bloque();
//    bwrite(ptr,bufferBloqueIndice);
//    bloque3 = bufferBloqueIndice[120];
//
//    escribir_inodo(inodo,ninodo);

//    printf("El bloque fisico %u esta mapeado por el bloque logico %u\n",bufferBloqueIndice[120],400004);
//    printf("Mapeado: inodo.punteroIndirecto -> %d.indice5 -> %d.indice25 -> %d.indice120 -> %d\n",bloque0,bloque1,bloque2,bloque3);
//    unsigned int nbloquefisico = traducir_bloque_inodo(ninodo, 400004, 0);
//    printf("El bloque fisico resulto es %u\n",nbloquefisico);

    puts("INODO CON BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 16.843.019");
    int ninodo=reservar_inodo('f', 6);
    printf ("ninodo: %d\n", ninodo);
    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 8, 1));
    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 204, 1));
    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 30004, 1));
    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 400004, 1));
    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 16843019, 1));

    inodo_t inodo;
    leer_inodo(ninodo,&inodo);
    puts("");
    printf("Tipo: %c\n",inodo.tipo);
    printf("Permisos: %u\n",inodo.permisos);
    //printf("atime: %d\n",inodoRaiz.atime); // todo: imprimirlo en el formato correspondiente
    //printf("mtime: %d\n",inodoRaiz.mtime);
    //printf("ctime: %d\n",inodoRaiz.ctime);
    printf("nlinks: %u\n",inodo.nlinks);
    printf("tamEnBytesLog: %u\n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",inodo.numBloquesOcupados);
    puts("");

    if(bumount() == -1){
        return -1;
    }

    return 0;
}
