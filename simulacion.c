#include <sys/wait.h>
#include <signal.h>
#include "directorios/directorios.h"

#define NPROCESOS  100
#define POS_MAX 50000
#define SLEEP_PROCESS 0
#define SLEEP_WRITE 0

int acabados = 0;

void reaper(){
    while(wait3(NULL, WNOHANG, NULL) > 0)
        acabados++;
}

int write_randomly(char *filepath){
    int resultado;
    struct registro reg;
    reg.pid = getpid();
    srand(time(NULL)+reg.pid);

    for (int i = 0; i < 50; ++i) {
        reg.nEscritura = i;
        reg.time = time(NULL);
        reg.posicion=(rand()%POS_MAX)* sizeof(struct registro); // multiplo de sizeof(registro)

//        printf("%u escribe registro %u en posicion %u en el instante %lu\n",reg.pid,reg.nEscritura,reg.posicion,reg.time);

        resultado = mi_write(filepath, &reg, reg.posicion, sizeof(struct registro));
        if(resultado < 0){
            return resultado;
        }

        usleep(SLEEP_WRITE);
    }

    return 0;
}

int writer(int id, char *dir) {
    char *filepath = (char *) malloc(100);
    sprintf(filepath,"%sproceso_%d/",dir,getpid());
    int resultado;

    //printf("%d creando el directorio '%s'\n", getpid(), filepath);
    resultado = mi_creat(filepath, 7);
    if(resultado < 0){
        return resultado;
    }

    strcat(filepath,"prueba.dat");
//    printf("%d creando el fichero '%s'\n", getpid(), filepath);
    resultado = mi_creat(filepath, 7);
    if(resultado < 0){
        return resultado;
    }

    resultado = write_randomly(filepath);
    if(resultado < 0){
        return resultado;
    }

    free(filepath);

    return 0;
}

void forker(char *fsdisk, int id, char *dir) {
    pid_t pid = fork();

    if (pid < 0)
    {
        fprintf(stderr,"Error creando proceso %d\n",id);
        exit(-1);
    }
    else if (pid == 0)
    {
        bmount(fsdisk);
//        printf("Child %d lanzado\n",id);
        writer(id, dir);
//        printf("Child %d terminado\n",id);
        bumount();
        exit(0);
    }

    usleep(SLEEP_PROCESS);
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <disco>\n",argv[0]);
        return -1;
    }

    signal(SIGCHLD, reaper);
    bmount(argv[1]);

    // Creamos el directorio de simulacion
    time_t now;
    time(&now);
    struct tm *ts = localtime(&now);
    char str_time[80];
    strftime(str_time, 80, "simul_%Y%m%d%H%M%S", ts); // todo SIZEOF?
    char dir_simulacion[80];
    sprintf(dir_simulacion,"/%s/",str_time);
    mi_creat(dir_simulacion,7);
    bumount();

    usleep(100000);
    // Lanzamos los procesos
    for (int i = 0; i < NPROCESOS; ++i) {
        forker(argv[1], i, dir_simulacion);
    }

//    wait(NULL);
    while (acabados < NPROCESOS){
        pause();
    }

//    bumount();

    printf("%s\n",dir_simulacion);

    exit(0);
}
