#include "directorios/directorios.h"

#define NUM_ENTRADAS_PROCESOS 100

void imprimirRegistro(struct registro reg){
    fprintf(stderr,"pos: %lu, ",reg.posicion / sizeof(struct registro));
    fprintf(stderr,"nescritura: %u\n",reg.nEscritura);
    fprintf(stderr,"Time: %s",asctime(localtime(&reg.time)));
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
        exit(-1);
    }
    if (n_entradas_leidas != NUM_ENTRADAS_PROCESOS){
        fprintf(stderr,"El directorio debe tener %u entradas exactamente, tiene %u\n", NUM_ENTRADAS_PROCESOS, n_entradas_leidas);
        exit(-1);
    }

    char archivo_informe[strlen(camino)+20];
    unsigned int offsetInforme = 0;
    sprintf(archivo_informe, "%s/informe.txt", camino);
    if (mi_creat(archivo_informe, 6) < 0) fprintf(stderr,"Error al crear el fichero\n");

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

                    if(buffer_registros[i].time < primera_escritura.time){
                        primera_escritura = buffer_registros[i];
                    } else if(buffer_registros[i].time > ultima_escritura.time){
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

        // Impresion por pantalla
        fprintf(stderr,"-----------Proceso %u------------ \n",pid);
        fprintf(stderr,"Primera escritura: ");
        imprimirRegistro(primera_escritura);
        fprintf(stderr,"Ultima escritura: ");
        imprimirRegistro(ultima_escritura);
        fprintf(stderr,"Mayor Posicion: ");
        imprimirRegistro(mayor_pos);
        fprintf(stderr,"Menor Posicion: ");
        imprimirRegistro(menor_pos);
        fprintf(stderr,"Numero de registros validos: %u\n", num_registros_validos);
        fprintf(stderr,"--------------------------");
        fprintf(stderr,"\n");

        // Impresion en informe.txt
        unsigned int tamBuffer2 = 1000;
        char bufferAux[tamBuffer2], buffer[tamBuffer2];
        *buffer = '\0';
        *bufferAux = '\0';

        sprintf(bufferAux, "-----------Proceso %u------------ \n", pid);
        strcat(buffer,bufferAux);

        strcat(buffer,"Primera escritura: ");
        sprintf(bufferAux,"pos: %lu, ",primera_escritura.posicion / sizeof(struct registro));
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"nescritura: %u\n",primera_escritura.nEscritura);
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"Time: %s",asctime(localtime(&primera_escritura.time)));
        strcat(buffer,bufferAux);

        strcat(buffer,"Ultima escritura: ");
        sprintf(bufferAux,"pos: %lu, ",ultima_escritura.posicion / sizeof(struct registro));
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"nescritura: %u\n",ultima_escritura.nEscritura);
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"Time: %s",asctime(localtime(&ultima_escritura.time)));
        strcat(buffer,bufferAux);

        strcat(buffer,"Mayor Posicion: ");
        sprintf(bufferAux,"pos: %lu, ",mayor_pos.posicion / sizeof(struct registro));
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"nescritura: %u\n",mayor_pos.nEscritura);
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"Time: %s",asctime(localtime(&mayor_pos.time)));
        strcat(buffer,bufferAux);

        strcat(buffer,"Menor Posicion: ");
        sprintf(bufferAux,"pos: %lu, ",menor_pos.posicion / sizeof(struct registro));
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"nescritura: %u\n",menor_pos.nEscritura);
        strcat(buffer,bufferAux);
        sprintf(bufferAux,"Time: %s",asctime(localtime(&menor_pos.time)));
        strcat(buffer,bufferAux);

        sprintf(bufferAux, "Numero de registros validos: %u\n", num_registros_validos);
        strcat(buffer,bufferAux);

        strcat(buffer,"--------------------------\n");

        int resultado = mi_write(archivo_informe,buffer,offsetInforme,strlen(buffer));
        if(resultado < 0) return resultado;
        offsetInforme += resultado;
    }

    bumount();

    return 0;
}