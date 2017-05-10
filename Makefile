CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=-pthread

SOURCES=bloques/bloques.c ficheros_basico/ficheros_basico.c ficheros/ficheros.c directorios/directorios.c mi_mkfs.c leer_sf.c mi_escribir.c mi_leer.c truncar.c permitir.c mi_mkdir mi_touch mi_chmod mi_ls mi_stat
LIBRARIES=bloques/bloques.o ficheros_basico/ficheros_basico.o ficheros/ficheros.o directorios/directorios.o
INCLUDES=bloques/bloques.h ficheros_basico/ficheros_basico.h ficheros/ficheros.h directorios/directorios.h
PROGRAMS=mi_mkfs leer_sf mi_escribir mi_leer truncar permitir mi_mkdir mi_touch mi_chmod mi_ls mi_stat
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

mi_mkfs: mi_mkfs.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

leer_sf: leer_sf.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_escribir: mi_escribir.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_leer: mi_leer.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

truncar: truncar.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

permitir: permitir.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_mkdir: mi_mkdir.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_touch: mi_touch.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_chmod: mi_chmod.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_ls: mi_ls.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_stat: mi_stat.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS)
	rm -rf bloques/*.o
	rm -rf ficheros_basico/*.o
	rm -rf ficheros/*.o
	rm -rf directorios/*.o
