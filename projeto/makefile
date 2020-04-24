CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-contab.o pingpong-contab-prio.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: noprio prio

noprio: pingpong-contab.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

prio: pingpong-contab-prio.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) noprio prio