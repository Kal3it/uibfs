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

/**
 *
 * @param ninodo
 * @param buf_original Stream a escribir
 * @param offset Byte desde el cual se escribirá (incluido)
 * @param nbytes Cantidad de bytes a escribir (Se escribira desde el byte (offset) hasta el byte (nbytes-1)
 * @return Cantidad de bytes escritos
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 *
 * @param ninodo
 * @param buf_original Buffer al cual se volcarán los datos leídos
 * @param offset Byte desde el cual se leerá
 * @param nbytes Cantidad de bytes a leer (Se leera desde el byte (offset) hasta el byte (nbytes-1)
 * @return Cantidad de bytes leídos
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 * p_stat tendrá la información de $ninodo
 * @param ninodo
 * @param p_stat
 * @return
 */
int mi_stat_f(unsigned int ninodo, stat_t *p_stat);

/**
 * Cambia los permisos de $ninodo a $permisos
 * @param ninodo
 * @param permisos
 * @return
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

/**
 * Trunca el ninodo a nbytes (su último byte será (nbytes-1))
 * @param ninodo
 * @param nbytes
 * @return
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);
