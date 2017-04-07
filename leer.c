#include "ficheros/ficheros.h"

int main(int argc, char const *argv[]){
    if (argc != 5) {
        puts("Uso: escribir <nombre_dispositivo> <ninodo> <offset> <nbytes>");
        return -1;
    }

    bmount(argv[1]);

    char buffer[atoi(argv[4])];
    mi_read_f(atoi(argv[2]),buffer,atoi(argv[3]),atoi(argv[4]));

    for (int i = 0; i < atoi(argv[4]); ++i) {
        printf("%c",buffer[i]);
    }
    puts("");

    bumount();

    return 0;
}