CC=gcc
CFLAGS=-I. -Wall -g -lm
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-mqueue.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: mqueue

mqueue: pingpong-mqueue.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) mqueue
