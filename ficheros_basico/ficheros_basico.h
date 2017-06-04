#include <time.h>
#include <limits.h>
#include "../bloques/bloques.h"

#define posSB 0 //el superbloque se escribe en el primer bloque de nuestro FS
#define T_INODO 128 //tamaño en bytes de un inodo (debe ser igual a sizeof(inodo_t))

#define DIRECTOS 12
#define NPUNTEROS           (int) (BLOCKSIZE/sizeof(unsigned int))
#define NPUNTEROS_CUADRADO  NPUNTEROS * NPUNTEROS
#define NPUNTEROS_CUBO      NPUNTEROS * NPUNTEROS * NPUNTEROS
#define TAM_MAX_FICHERO     (DIRECTOS + NPUNTEROS + NPUNTEROS_CUADRADO + NPUNTEROS_CUBO) * BLOCKSIZE

#define TIPO_FICHERO 'f'
#define TIPO_DIRECTORIO 'd'
#define TIPO_ENLACE 'l'
#define TIPO_LIBRE '-'

#define BLOQUE_LOGICO_NO_INICIALIZADO -5
#define NO_QUEDAN_BLOQUES_LIBRES -6
#define NO_QUEDAN_INODOS_LIBRES -7
#define PERMISOS_INVALIDOS -8
#define BLOQUE_LOGICO_FUERA_DE_RANGO -9

struct superbloque{
    unsigned int posPrimerBloqueMB;     //Posición del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB;     //Posición del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI;     //Posición del primer bloque del array de inodos
    unsigned int posUltimoBloqueAI;     //Posición del último bloque del array de inodos
    unsigned int posPrimerBloqueDatos;  //Posición del primer bloque de datos
    unsigned int posUltimoBloqueDatos;  //Posición del último bloque de datos
    unsigned int posInodoRaiz;          //Posición del inodo del directorio raíz
    unsigned int posPrimerInodoLibre;   //Posición del primer inodo libre
    unsigned int cantBloquesLibres;     //Cantidad de bloques libres
    unsigned int cantInodosLibres;      //Cantidad de inodos libres
    unsigned int totBloques;            //Cantidad total de bloques
    unsigned int totInodos;             //Cantidad total de inodos
    char padding[BLOCKSIZE-12*sizeof(unsigned int)]; //Relleno
};

typedef union _inodo{
    struct {
        unsigned char tipo; //Tipo (libre, directorio o fichero)
        unsigned char permisos; //Permisos (lectura y/o escritura y/o ejecución)

        time_t atime; //Fecha y hora del último acceso a datos: atime
        time_t mtime; //Fecha y hora de la última modificación de datos: mtime
        time_t ctime; //Fecha y hora de la última modificación del inodo: ctime

        /* comprobar el tamaño del tipo time_t para vuestra plataforma/compilador: printf ("sizeof time_t is: %d\n", sizeof(time_t)); */

        unsigned int nlinks; //Cantidad de enlaces de entradas en directorio
        unsigned int tamEnBytesLog; //Tamaño en bytes lógicos
        unsigned int numBloquesOcupados; //Cantidad de bloques ocupados en la zona de datos

        unsigned int punterosDirectos[12]; //12 punteros a bloques directos
        unsigned int punterosIndirectos[3]; /*3 punteros a bloques indirectos:1 puntero indirecto simple, 1 puntero indirecto doble, 1 puntero indirecto triple */
    };
    char padding[T_INODO];
} inodo_t;

/**
 * @param nbloques
 * @return tamaño del mapa de bits en bloques.
 */
int tamMB (unsigned int nbloques);

/**
 * @param ninodos
 * @return tamaño del array de inodos en bloques.
 */
int tamAI (unsigned int ninodos);

/**
 * Inicia los atributos del superbloque.
 * @param nbloques Número de bloques total
 * @param ninodos Número de inodos total
 * @return
 */
int initSB(unsigned int nbloques, unsigned int ninodos);

/**
 * Inicia los atributos del mapa de bits.
 * @param nbloques
 * @return
 */
int initMB(unsigned int nbloques);

/**
 * Inicia el array de inodos creando inodos libres.
 * @param ninodos
 * @return
 */
int initAI(unsigned int ninodos);

/**
 * Escribe el valor $bit en la posición correspondiente del bloque $nbloque en el mapa de bits.
 * @param nbloque
 * @param bit
 * @return
 */
int escribit_bit(unsigned int nbloque, unsigned int bit);

/**
 * Devuelve el valor del bit del mapa de bits correspondiente al bloque $nbloque.
 * @param nbloque
 * @return
 */
unsigned char leer_bit(unsigned int nbloque);

/**
 * Marca como ocupado el primer bloque no ocupado y devuelve la direccion fisica de éste
 * @return
 */
int reservar_bloque();

/**
 * Marca como no ocupado el bloque $nbloque
 * @param nbloque
 * @return
 */
int liberar_bloque(unsigned int nbloque);

/**
 * Escribe el inodo $inodo en la posicion $ninodo del array de inodos.
 * @param inodo
 * @param ninodo
 * @return
 */
int escribir_inodo(inodo_t inodo, unsigned int ninodo);

/**
 * Lee el inodo de la posicion $ninodo del array de inodos y lo apunta con $inodo
 * @param ninodo
 * @param inodo
 * @return
 */
int leer_inodo(unsigned int ninodo, inodo_t * inodo);

/**
 * Reserva una posicion de inodo libre y devuelve ésta
 * @param tipo
 * @param permisos
 * @return
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos);

/**
 * Dado un ninodo y el bloque logico de éste, devuelve el bloque físico asociado.
 * @param ninodo
 * @param nblogico
 * @param reservar Inicializa el bloque si no lo está
 * @return
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar);

/**
 * Libera todos los bloques logicos del inodo numero $ninodo a partir de $nblogico
 * @param ninodo
 * @param nblogico
 * @return
 */
int liberar_bloques_inodo(unsigned int ninodo, unsigned int nblogico);

int liberar_inodo(unsigned int ninodo);