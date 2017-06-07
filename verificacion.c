#include "directorios/directorios.h"

#define NUM_ENTRADAS_PROCESOS 100

void imprimirRegistro(struct registro reg){
    printf("Time: %s\n",asctime(localtime(&reg.time)));
    printf("PID: %u, ",reg.pid);
    printf("Posicion: %u, ",reg.posicion);
    printf("Numero de escritura: %u\n",reg.nEscritura);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        puts("uso: ./simulacion <disco> <directorio de simulacion>");
        return -1;
    }

    bmount(argv[1]);

    const char *camino = argv[2];

    entrada_t buffer_entradas[NUM_ENTRADAS_PROCESOS];
    int n_entradas_leidas = mi_dir_simple(camino,&buffer_entradas);
    if (n_entradas_leidas < 0)
    {
        fprintf(stderr,"Error leyendo entradas del directorio\n");
        return -1;
    }
    if (n_entradas_leidas != NUM_ENTRADAS_PROCESOS){
        fprintf(stderr,"El directorio debe tener %u entradas exactamente, tiene %d\n", NUM_ENTRADAS_PROCESOS, n_entradas_leidas);
        return -1;
    }

    for (int n_fichero_proceso = 0; n_fichero_proceso < NUM_ENTRADAS_PROCESOS; ++n_fichero_proceso) {
        char fichero_registros[strlen(camino)+strlen(buffer_entradas[n_fichero_proceso].nombre)+10];

        char *ptr_pid = strchr(buffer_entradas[n_fichero_proceso].nombre,'_'); //extraemos el pid del nombre de la entrada
        pid_t pid = atoi(ptr_pid+1);

        sprintf(fichero_registros, "%s/%s/prueba.dat", camino, buffer_entradas[n_fichero_proceso].nombre);

        struct registro primera_escritura, ultima_escritura, mayor_pos, menor_pos;

        unsigned int tamBuffer = 500 * sizeof(struct registro);
        unsigned int offset = 0, num_registros_validos = 0;
        struct registro buffer_registros[tamBuffer/ sizeof(struct registro)];
        memset(buffer_registros,0,tamBuffer);

        int respuesta = mi_read(fichero_registros, buffer_registros, offset, tamBuffer);

        while(respuesta > 0){

            for (int i = 0; i < respuesta/ sizeof(struct registro); ++i) {
                if(buffer_registros[i].pid == pid){

                    num_registros_validos++;

                    if(num_registros_validos == 1){
                        primera_escritura = buffer_registros[i];
                        ultima_escritura = buffer_registros[i];
                        mayor_pos = buffer_registros[i];
                        menor_pos = buffer_registros[i];
                        continue;
                    }

                    if(buffer_registros[i].nEscritura < primera_escritura.nEscritura){
                        primera_escritura = buffer_registros[i];
                    } else if(buffer_registros[i].nEscritura > ultima_escritura.nEscritura){
                        ultima_escritura = buffer_registros[i];
                    }

                    if(buffer_registros[i].posicion < menor_pos.posicion){
                        menor_pos = buffer_registros[i];
                    }else if(buffer_registros[i].posicion > mayor_pos.posicion){
                        mayor_pos = buffer_registros[i];
                    }

                }
            }

            offset += tamBuffer;
            memset(buffer_registros,0,tamBuffer);
            respuesta = mi_read(fichero_registros, buffer_registros, offset, tamBuffer);
        }

        printf("Proceso %u\n",pid);
        puts("Primera escritura");
        imprimirRegistro(primera_escritura);
        puts("Ultima escritura");
        imprimirRegistro(ultima_escritura);
        puts("Mayor Posicion");
        imprimirRegistro(mayor_pos);
        puts("Menor Posicion");
        imprimirRegistro(menor_pos);
        printf("Numero de registros validos: %u\n", num_registros_validos);
        puts("");

        // TODO informe
    }

    bumount();

    return 0;
}