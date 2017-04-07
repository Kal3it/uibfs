#include "../ficheros_basico/ficheros_basico.h"

#define PERMISOS_INSUFICIENTES -10
#define ACCESO_FUERA_DE_RANGO -11

typedef struct STAT {
    unsigned char tipo;
    unsigned char permisos;

    time_t atime; //Fecha y hora del último acceso a datos: atime
    time_t mtime; //Fecha y hora de la última modificación de datos: mtime
    time_t ctime; //Fecha y hora de la última modificación del inodo: ctime

    unsigned int nlinks;
    unsigned int tamEnBytesLog;
    unsigned int numBloquesOcupados;
} stat_t;

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);

int mi_stat_f(unsigned int ninodo, stat_t *p_stat);

int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);
