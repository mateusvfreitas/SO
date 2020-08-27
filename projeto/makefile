CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-prodcons.o queue.o pingpong-racecond.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: prodcons racecond

prodcons: pingpong-prodcons.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

racecond: pingpong-racecond.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) prodcons racecond
