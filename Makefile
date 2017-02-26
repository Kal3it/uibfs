CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=-pthread

SOURCES=etapa1/bloques.c etapa2/ficheros_basico.c mi_mkfs.c leer_sf.c
LIBRARIES=etapa1/bloques.o etapa2/ficheros_basico.o
INCLUDES=etapa1/bloques.h etapa2/ficheros_basico.h
PROGRAMS=mi_mkfs leer_sf
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

mi_mkfs: mi_mkfs.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

leer_sf: leer_sf.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LIBRARIES) $< -o $@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS)
