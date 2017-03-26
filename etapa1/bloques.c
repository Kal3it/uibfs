#include "bloques.h"
static int descriptor=-1;

int bmount(const char *camino){
	descriptor=open(camino,O_RDWR|O_CREAT,0666);
	if(descriptor==-1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
	return 0;
}

int bumount(){
	if(close(descriptor)==-1){
		fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
		exit(-1);
	}
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