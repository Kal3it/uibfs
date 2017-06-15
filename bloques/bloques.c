#include "bloques.h"
static int descriptor=-1;
static unsigned int inside_sc = 0;
static sem_t *mutex;

int bmount (const char *camino){
	if (descriptor > 0) {
		close(descriptor);
	}
	if ((descriptor = open(camino, O_RDWR|O_CREAT, 0666)) == -1) {
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));}
	if (!mutex) {
		mutex = initSem();
		if (mutex == SEM_FAILED) {
			return -1;
		}
	}
	return 0;
}

int bumount(){
	if(close(descriptor)==-1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	deleteSem();
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

void mi_waitSem(){
    if (!inside_sc) {
        waitSem(mutex);
    }
    inside_sc++;
}
void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}