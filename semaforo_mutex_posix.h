#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define SEM_FICHEROS 0
#define SEM_DIRECTORIOS 1

int initSem(int sem_level);
void deleteSem(int sem_level);
void signalSem(int sem_level);
void waitSem(int sem_level);