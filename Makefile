CC=gcc
CFLAGS=-c -g -Wall -std=gnu99
LDFLAGS=-pthread

SOURCES=bloques/bloques.c ficheros_basico/ficheros_basico.c ficheros/ficheros.c directorios/directorios.c mi_mkfs.c leer_sf.c mi_escribir.c mi_cat.c truncar.c permitir.c mi_mkdir.c mi_touch.c mi_chmod.c mi_ls.c mi_stat.c mi_cp.c mi_link.c mi_rm.c mi_mv.c semaforo_mutex_posix.c simulacion.c verificacion.c leer.c escribir.c
LIBRARIES=bloques/bloques.o ficheros_basico/ficheros_basico.o ficheros/ficheros.o directorios/directorios.o semaforo_mutex_posix.o
INCLUDES=bloques/bloques.h ficheros_basico/ficheros_basico.h ficheros/ficheros.h directorios/directorios.h semaforo_mutex_posix.h
PROGRAMS=mi_mkfs leer_sf mi_escribir mi_cat truncar permitir mi_mkdir mi_touch mi_chmod mi_ls mi_stat mi_cp mi_link mi_rm mi_mv simulacion verificacion leer escribir
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(SOURCES) $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS)
	rm -rf bloques/*.o
	rm -rf ficheros_basico/*.o
	rm -rf ficheros/*.o
	rm -rf directorios/*.o
