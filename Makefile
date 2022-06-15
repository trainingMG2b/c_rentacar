IDIR =include
CC=gcc
CFLAGS=-I$(IDIR) -std=c99 -g

SDIR=src
ODIR=obj
LDIR=lib

LIBS=-lm

_DEPS = rentacar.h car.h cJSON.h data.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rentacar.o car.o cJSON.o data.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rentacar: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

all: clean rentacar
	@echo 'DEPS = $(DEPS)'
	@echo 'OBJ  = $(OBJ)'

default: all
