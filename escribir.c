#include "ficheros/ficheros.h"

void imprimirStat(stat_t *stat){
    fprintf(stderr,"stat.tamEnBytesLog: %u\n",stat->tamEnBytesLog);
    fprintf(stderr,"stat.numBloquesOcupados: %u\n",stat->numBloquesOcupados);

    fprintf(stderr,"\n");
}

//int main(int argc, char const *argv[]){
//
//    unsigned int numoffsets = 1;
//    unsigned int offsets[numoffsets];
//    offsets[0] = 0;
////    offsets[1] = 5120;
////    offsets[2] = 256000;
////    offsets[3] = 30720000;
////    offsets[4] = 71680000;
//
//    if (argc != 4) {
//        puts("Uso: escribir <nombre_dispositivo> <fichero> <diferentes_inodos>");
//        fprintf(stderr,"Offsets: ");
//        for (int i = 0; i < numoffsets; ++i) {
//            fprintf(stderr,"%u ",offsets[i]);
//        }
//        fprintf(stderr,"\n");
//        puts("Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
//        return -1;
//    }
//
//    stat_t stat;
//    unsigned int numinodos = atoi(argv[3]) ? numoffsets : 1, bytesEscritos = 0;
//    const char *fichero = argv[2];
//    char c;
//
//    bmount(argv[1]);
//
//    int ninodos[numinodos];
//    for (int i = 0; i < numinodos; ++i) {
//        // Reserva inodo
//        ninodos[i] = reservar_inodo('f',6);
//        fprintf(stderr,"Nº inodo reservado: %u: %u\n",i,ninodos[i]);
//
//        // Escribe secuencial
//        if(numinodos == 1){
//            for (int j = 0; j < numoffsets; ++j) {
//                fprintf(stderr,"offset: %u\n",offsets[j]);
//                FILE *fd = fopen(fichero,"r");
//                c = fgetc(fd);
//                for (int k = 0; c != EOF; ++k) {
//                    bytesEscritos += mi_write_f(ninodos[i],&c,offsets[j]+k,1); // todos los offset para un inodo
//                    if(bytesEscritos == PERMISOS_INSUFICIENTES) break;
//                    c = fgetc(fd);
//                }
//                if(bytesEscritos == PERMISOS_INSUFICIENTES) break;
//                fprintf(stderr,"Bytes escritos: %u\n",bytesEscritos);
//                bytesEscritos = 0;
//                fclose(fd);
//                // Imprime stat
//                mi_stat_f(ninodos[i],&stat);
//                imprimirStat(&stat);
//            }
//
//        } else {
//            FILE *fd = fopen(fichero,"r");
//            c = fgetc(fd);
//            for (int j = 0; c != EOF; ++j) {
//                bytesEscritos += mi_write_f(ninodos[i],&c,offsets[i]+j,1); // cada offset para cada inodo
//                if(bytesEscritos == PERMISOS_INSUFICIENTES) break;
//                c = fgetc(fd);
//            }
//            fclose(fd);
//            fprintf(stderr,"offset: %u\n",offsets[i]);
//            fprintf(stderr,"Bytes escritos: %u\n",bytesEscritos);
//            bytesEscritos = 0;
//
//            // Imprime stat
//            mi_stat_f(ninodos[i],&stat);
//            imprimirStat(&stat);
//        }
//    }
//
//    bumount();
//
//    return 0;
//}

int main(int argc, char const *argv[]){

    unsigned int numoffsets = 5;
    unsigned int offsets[numoffsets];
    offsets[0] = 0;
    offsets[1] = 5120;
    offsets[2] = 256000;
    offsets[3] = 30720000;
    offsets[4] = 71680000;

    if (argc != 4) {
        puts("Uso: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>");
        fprintf(stderr,"Offsets: ");
        for (int i = 0; i < numoffsets; ++i) {
            fprintf(stderr,"%u ",offsets[i]);
        }
        fprintf(stderr,"\n");
        puts("Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
        return -1;
    }

    stat_t stat;
    unsigned int inodos_diferentes = atoi(argv[3]);
    unsigned int num_inodos = inodos_diferentes ? numoffsets : 1; // 1 inodo o tantos como offsets
    int ninodos[num_inodos];
    const char *buffer = argv[2];
    const int len = strlen(buffer);

    bmount(argv[1]);

    for (int i = 0; i < num_inodos; ++i) {
        // Reserva inodos (1 o N)
        ninodos[i] = reservar_inodo('f',6);
        fprintf(stderr,"Nº inodo reservado %u: %u\n",i,ninodos[i]);
    }

    for (int i = 0, bytesEscritos = 0; i < numoffsets; ++i, bytesEscritos = 0) {

        fprintf(stderr,"offset: %u\n",offsets[i]);
        if(inodos_diferentes)   bytesEscritos += mi_write_f(ninodos[i],buffer,offsets[i],len); // diferente offset para diferente inodo
        else                    bytesEscritos += mi_write_f(ninodos[0],buffer,offsets[i],len); // todos los offset para un inodo

        if(bytesEscritos == PERMISOS_INSUFICIENTES) break;
        fprintf(stderr,"Bytes escritos: %u\n",bytesEscritos);

        // Imprime stat
        if(inodos_diferentes)   mi_stat_f(ninodos[i],&stat);
        else                    mi_stat_f(ninodos[0],&stat);

        imprimirStat(&stat);
    }

    bumount();

    return 0;
}