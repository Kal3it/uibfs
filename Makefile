CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=-pthread

SOURCES=bloques/bloques.c ficheros_basico/ficheros_basico.c ficheros/ficheros.c directorios/directorios.c mi_mkfs.c leer_sf.c mi_escribir.c mi_cat.c truncar.c permitir.c mi_mkdir.c mi_touch.c mi_chmod.c mi_ls.c mi_stat.c mi_cp.c mi_link.c mi_rm.c
LIBRARIES=bloques/bloques.o ficheros_basico/ficheros_basico.o ficheros/ficheros.o directorios/directorios.o
INCLUDES=bloques/bloques.h ficheros_basico/ficheros_basico.h ficheros/ficheros.h directorios/directorios.h
PROGRAMS=mi_mkfs leer_sf mi_escribir mi_cat truncar permitir mi_mkdir mi_touch mi_chmod mi_ls mi_stat mi_cp mi_link mi_rm
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

mi_cat: mi_cat.o $(LIBRARIES) $(INCLUDES)
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

mi_cp: mi_cp.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_link: mi_link.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

mi_rm: mi_rm.o $(LIBRARIES) $(INCLUDES)
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
