#include <sys/wait.h>
#include <signal.h>
#include "directorios/directorios.h"

#define NPROCESOS  80
#define POS_MAX 50000

int acabados = 0;

void reaper(){
    while(wait3(NULL, WNOHANG, NULL) > 0)
        acabados++;
}

void write_randomly(char *filepath){
    struct registro reg;
    reg.pid = getpid();
    srand(time(NULL)+reg.pid);

    for (int i = 0; i < 50; ++i) {
        reg.nEscritura = i;
        reg.time = time(NULL);
        reg.posicion=(rand()%POS_MAX)* sizeof(struct registro); // multiplo de sizeof(registro)

        //fprintf(stderr,"%u escribe registro %u en posicion %u en el instante %lu\n",reg.pid,reg.nEscritura,reg.posicion,reg.time);

        mi_write(filepath, &reg, reg.posicion, sizeof(struct registro));

        usleep(2000);
    }
}

void writer(int id, char *dir) {
    char *filepath = (char *) malloc(100);
    sprintf(filepath,"%sproceso_%d/",dir,getpid());

    mi_creat(filepath, 7);
    //printf("%d ha creado el directorio '%s'\n", getpid(), filepath);
    strcat(filepath,"prueba.dat");
    mi_creat(filepath, 7);
    //printf("%d ha creado el fichero '%s'\n", getpid(), filepath);

    write_randomly(filepath);

    free(filepath);
}

void forker(int id, char *dir) {
    pid_t pid = fork();

    if (pid < 0)
    {
        fprintf(stderr,"Error creando proceso %d\n",id);
        exit(-1);
    }
    else if (pid == 0)
    {
        printf("Child %d lanzado\n",id);
        writer(id, dir);
        printf("Child %d terminado\n",id);
        exit(0);
    }
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <disco>\n",argv[0]);
        return -1;
    }

    signal(SIGCHLD, reaper);
    bmount(argv[1]);

    // Creamos el directorio de simulacion
    char str_time[80];
    time_t now;
    time(&now);
    struct tm *ts = localtime(&now);
    strftime(str_time, sizeof(str_time), "simul_%Y%m%d%H%M%S", ts); // todo SIZEOF?
    char dir_simulacion[80];
    sprintf(dir_simulacion,"/%s/",str_time);
    printf("%s\n",dir_simulacion);
    mi_creat(dir_simulacion,7);

    // Lanzamos los procesos
    for (int i = 0; i < NPROCESOS; ++i) {
        forker(i, dir_simulacion);
        wait(NULL);
    }

    wait(NULL);
//    while (acabados < NPROCESOS){
//        pause();
//    }

    bumount();

    return 0;
}
