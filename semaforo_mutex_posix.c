#include "semaforo_mutex_posix.h"

#define SEM_FICHEROS_NAME "/mutex_ficheros"
#define SEM_DIRECTORIOS_NAME "/mutex_directorios"
#define SEM_INIT_VALUE 1 /* Valor inicial de los mutex */

static sem_t *mutex_ficheros;
static sem_t *mutex_directorios;

/* Ejemplo de creación e inicialización de semáforos POSIX para MUTEX con "semáforos con nombre" (named) */

int initSem(int sem_level) {

    if((sem_level == SEM_FICHEROS && mutex_ficheros) || (sem_level == SEM_DIRECTORIOS && mutex_directorios)){
        return 0;
    }

    switch (sem_level){
        case SEM_FICHEROS:
//            printf("Se abre semaforo %d\n",sem_level);
            mutex_ficheros = sem_open(SEM_FICHEROS_NAME, O_CREAT, S_IRWXU, SEM_INIT_VALUE);
            if (mutex_ficheros == SEM_FAILED) {
                fprintf(stderr,"Error creando el semaforo %d\n",sem_level);
                return -1;
            }
            break;
        case SEM_DIRECTORIOS:
//            printf("Se abre semaforo %d\n",sem_level);
            mutex_directorios = sem_open(SEM_DIRECTORIOS_NAME, O_CREAT, S_IRWXU, SEM_INIT_VALUE);
            if (mutex_directorios == SEM_FAILED) {
                fprintf(stderr,"Error creando el semaforo %d\n",sem_level);
                return -1;
            }
            break;
    }
    return 0;
}

void deleteSem(int sem_level) {
//    printf("Se borra semaforo %d\n",sem_level);
    switch (sem_level){
        case SEM_FICHEROS:
            sem_unlink(SEM_FICHEROS_NAME);
            break;
        case SEM_DIRECTORIOS:
            sem_unlink(SEM_DIRECTORIOS_NAME);
            break;
    }
}

void signalSem(int sem_level) {
//    printf("Signal semaforo %d\n",sem_level);
    switch (sem_level){
        case SEM_FICHEROS:
            sem_post(mutex_ficheros);
            break;
        case SEM_DIRECTORIOS:
            sem_post(mutex_directorios);
            break;
    }
}

void waitSem(int sem_level) {
//    printf("Wait semanforo %d\n",sem_level);
    switch (sem_level){
        case SEM_FICHEROS:
            sem_wait(mutex_ficheros);
            break;
        case SEM_DIRECTORIOS:
            sem_wait(mutex_directorios);
            break;
    }
}
