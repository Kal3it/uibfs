CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=-pthread

SOURCES=bloques/bloques.c ficheros_basico/ficheros_basico.c ficheros/ficheros.c mi_mkfs.c leer_sf.c escribir.c leer.c
LIBRARIES=bloques/bloques.o ficheros_basico/ficheros_basico.o ficheros/ficheros.o
INCLUDES=bloques/bloques.h ficheros_basico/ficheros_basico.h ficheros/ficheros.h
PROGRAMS=mi_mkfs leer_sf escribir leer
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

mi_mkfs: mi_mkfs.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

leer_sf: leer_sf.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

escribir: escribir.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

leer: leer.o $(LIBRARIES) $(INCLUDES)
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
