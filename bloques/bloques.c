#include "bloques.h"
static int descriptor=-1;

int bmount(const char *camino){
    initSem(SEM_FICHEROS);
    initSem(SEM_DIRECTORIOS);

	descriptor=open(camino,O_RDWR|O_CREAT,0666);
	if(descriptor==-1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	return 0;
}

//int bmount (const char *camino){
//	if (descriptor > 0) {
//		close(descriptor);
//	}
//	if ((descriptor = open(camino, O_RDWR|O_CREAT, 0666)) == -1) {
//		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));}
//	if (!mutex) {
//		mutex = initSem();
//		if (mutex == SEM_FAILED) {
//			return -1;
//		}
//	}
//	return 0;
//}

int bumount(){
	if(close(descriptor)==-1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	deleteSem(SEM_FICHEROS);
	deleteSem(SEM_DIRECTORIOS);
	return 0;
}

int bwrite(unsigned int nbloque, const void *buf){
	if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	if(write(descriptor,buf,BLOCKSIZE) == -1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}

	return 0;
}

int bread(unsigned int nbloque,void *buf){
	if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	int t=read(descriptor,buf,BLOCKSIZE);
	if(t == -1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	return 0;
}

void mi_waitSem(int sem_level) {
    waitSem(sem_level);
}

void mi_signalSem(int sem_level) {
    signalSem(sem_level);
}