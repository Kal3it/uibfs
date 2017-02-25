#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> /* Modos de apertura y función open()*/


#define BLOCKSIZE 1024 //bytes

/**
 * Solicita al sistema un nuevo descriptor dado un archivo. Si el archivo existe, lo abre en modo RW, si no, lo crea.
 * @param camino Ruta del archivo
 * @return [0] Ok, [-1] Error
 */
int bmount(const char *camino);

/**
 * Cierra el fichero actualmente abierto.
 * @return [0] Ok, [-1] Error
 */
int bumount();

/**
 * Escribe el contenido de buf en el bloque $nbloque.
 * Escribirá $BLOCKSIZE bytes.
 * @param nbloque
 * @param buf Debe tener memoria asignada.
 * @return [0] Ok, [-1] Error
 */
int bwrite(unsigned int nbloque, const void *buf);

/**
 * Lee el contenido del bloque $nbloque y lo vuelca en buf
 * Leerá $BLOCKSIZE bytes
 * @param nbloque Número de bloque
 * @param buf Debe tener memoria asignada.
 * @return Número de bytes leídos (Máximo $BLOCKSIZE)
 */
int bread(unsigned int nbloque, void *buf);


