#include "ficheros/ficheros.h"

void imprimirInodoInfo(unsigned int ninodo);

int main(int argc, char const *argv[]){
    if (argc != 4) {
        puts("Uso: escribir <nombre_dispositivo> <fichero> <offset>");
        return -1;
    }

    //TODO: Falta hacer mas tests

    unsigned int offset = atoi(argv[3]);
    const char *fichero = argv[2];

    bmount(argv[1]);

    int ninodo = reservar_inodo('f',6);
    fprintf(stderr,"ninodo: %u\n",ninodo);

    FILE *fd = fopen(fichero,"r");
    fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
    unsigned int size;
    char c = fgetc(fd);
    for(size = offset; c != EOF; ++size){
        mi_write_f(ninodo, &c, size, 1);
        c = fgetc(fd);
    }
    fclose(fd);
    //imprimirInodoInfo(ninodo);

    fprintf(stderr,"%u\n",size);
    char buffer[size-offset];
    mi_read_f(ninodo,buffer,offset,size-offset);
    for (int i = 0; i < size-offset; ++i) {
        printf("%c",buffer[i]);
    }

    bumount();

    return 0;
}

void imprimirInodoInfo(unsigned int ninodo){
    inodo_t inodo;
    leer_inodo(ninodo,&inodo);

    fprintf(stderr,"Tipo: %c\n",inodo.tipo);
    fprintf(stderr,"Permisos: %u\n",inodo.permisos);
    //fprintf(stderr,"atime: %d\n",inodoRaiz.atime); // todo: imprimirlo en el formato correspondiente
    //fprintf(stderr,"mtime: %d\n",inodoRaiz.mtime);
    //fprintf(stderr,"ctime: %d\n",inodoRaiz.ctime);
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
}