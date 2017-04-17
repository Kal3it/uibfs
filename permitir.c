#include "ficheros/ficheros.h"

void imprimirInodoInfo(unsigned int ninodo){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);

    fprintf(stderr,"INFORMACION DEL INODO: \n");
    fprintf(stderr,"Ninodo: %u\n",ninodo);
    fprintf(stderr,"Tipo: %c\n",inodo.tipo);
    fprintf(stderr,"Permisos: %u\n",inodo.permisos);
    fprintf(stderr,"atime: %s\n",ctime(&inodo.atime));
    fprintf(stderr,"mtime: %s\n",ctime(&inodo.mtime));
    fprintf(stderr,"ctime: %s\n",ctime(&inodo.ctime));
    fprintf(stderr,"nlinks: %u\n",inodo.nlinks);
    fprintf(stderr,"tamEnBytesLog: %u\n",inodo.tamEnBytesLog);
    fprintf(stderr,"numBloquesOcupados: %u\n",inodo.numBloquesOcupados);

    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[0]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[1]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[2]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[3]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[4]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[5]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[6]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[7]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[8]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[9]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[10]);
    fprintf(stderr,"punterosDirectos: %u\n",inodo.punterosDirectos[11]);
    fprintf(stderr,"punterosIndirectos0: %u\n",inodo.punterosIndirectos[0]);
    fprintf(stderr,"punterosIndirectos1: %u\n",inodo.punterosIndirectos[1]);
    fprintf(stderr,"punterosIndirectos2: %u\n",inodo.punterosIndirectos[2]);
    fprintf(stderr,"\n");
}

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: permitir <nombre_dispositivo> <ninodo> <permisos>");
        return -1;
    }

    bmount(argv[1]);

    unsigned int
            ninodo = atoi(argv[2]),
            permisos = atoi(argv[3]);

    mi_chmod_f(ninodo,permisos);

    imprimirInodoInfo(ninodo);

    bumount();

    return 0;
}