#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "etapa1/bloques.h"
#include "etapa2/ficheros_basico.h"


void imprimirInodoInfo(unsigned int ninodo);
void imprimirSbInfo();

int main(int argc, char const *argv[]) {

    if (argc != 2) {
        puts("Uso: leer_sf <nombre_dispositivo>");
        return -1;
    }

    bmount(argv[1]);

//    imprimirSbInfo();

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

//	puts("DATOS DEL DIRECTORIO RAIZ");
//    imprimirInodoInfo(0);
	
//	puts("MAPA DE BITS");
//	printf("valorSB: %u\n",leer_bit(posSB));
//	printf("valorPrimerBloqueMB: %u\n",leer_bit(sb.posPrimerBloqueMB));
//	printf("valorUltimoBloqueMB: %u\n",leer_bit(sb.posUltimoBloqueMB));
//	printf("valorPrimerBloqueAI: %u\n",leer_bit(sb.posPrimerBloqueAI));
//	printf("valorUltimoBloqueAI: %u\n",leer_bit(sb.posUltimoBloqueAI));
//	printf("valorPrimerBloqueDatos: %u\n",leer_bit(sb.posPrimerBloqueDatos));
//	printf("valorUltimoBloqueDatos: %u\n",leer_bit(sb.posUltimoBloqueDatos));
//    puts("");

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

//    puts("INODO CON BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 16.843.019");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 8, 1));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 204, 1));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 30004, 1));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 400004, 1));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 16843019, 1));
//    puts("---------------------");

//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 8, 0));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 204, 0));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 30004, 0));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 400004, 0));
//    puts("---------------------");
//    printf("ptr bloque fisico de datos: %u\n",traducir_bloque_inodo(ninodo, 16843019, 0));
//    puts("---------------------");

//    unsigned int ninodo = reservar_inodo('f',7);
//    printf("indoo reservado %u\n",ninodo);

    liberar_inodo(6);
    liberar_inodo(2);
    liberar_inodo(3);

//    traducir_bloque_inodo(ninodo, 1, 1);
//    traducir_bloque_inodo(ninodo, 5, 1);
//    traducir_bloque_inodo(ninodo, 7, 1);
//    traducir_bloque_inodo(ninodo, 9, 1);
//    traducir_bloque_inodo(ninodo, 10, 1);
//    traducir_bloque_inodo(ninodo, 168, 1);
//    traducir_bloque_inodo(ninodo, 404, 1);
//    traducir_bloque_inodo(ninodo, 405, 1);
//    traducir_bloque_inodo(ninodo, 405, 1);
//    traducir_bloque_inodo(ninodo, 900, 1);

//    liberar_inodo(10);
//    liberar_inodo(4);
//    liberar_inodo(5);

//    imprimirInodoInfo(ninodo);
    imprimirSbInfo();
    bumount();

    return 0;
}

void imprimirInodoInfo(unsigned int ninodo){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);

    puts("INFORMACION DEL INODO: ");
    printf("Tipo: %c\n",inodo.tipo);
    printf("Permisos: %u\n",inodo.permisos);
    //printf("atime: %d\n",inodoRaiz.atime); // todo: imprimirlo en el formato correspondiente
    //printf("mtime: %d\n",inodoRaiz.mtime);
    //printf("ctime: %d\n",inodoRaiz.ctime);
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
    puts("");
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