#include <string.h>
#include "../ficheros/ficheros.h"

#define MAX_ENTRADAS BLOCKSIZE/sizeof(entrada_t)
#define MAX_TAM_NOMBRE_ENTRADA 60
#define MAX_TAM_NOMBRE_ENTRADA_CACHE 512
#define TAM_CACHE 10

#define NO_ES_DIRECTORIO -100
#define NO_ES_FICHERO -101
#define PATHNAME_INVALIDO -102
#define NO_EXISTE_ENTRADA -103
#define YA_EXISTE_ENTRADA -104
#define IMPOSIBLE_BORRAR_ENTRADA -105

typedef struct entrada {
    char nombre[MAX_TAM_NOMBRE_ENTRADA];
    unsigned int ninodo;
} entrada_t;

typedef struct entrada_cache {
    char nombre[MAX_TAM_NOMBRE_ENTRADA_CACHE];
    unsigned int ninodo;
    time_t ultimo_hit;
} entrada_cache_t;

typedef struct cache {
    unsigned int num_entradas_cacheadas;
    int ultima_entrada_acertada;
    int entrada_mas_antigua;
    entrada_cache_t entradas[TAM_CACHE];
} cache_t;

/**
 * Crea el fichero/directorio
 * @param camino
 * @param permisos
 * @return 0 si OK, <0 si PERMISOS_INSUFICIENTES, NO_ES_DIRECTORIO, PATHNAME_INVALIDO, NO_EXISTE_ENTRADA, YA_EXISTE_ENTRADA
 */
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer);
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);