CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-barrier.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: barrier

barrier: pingpong-barrier.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) barrier
