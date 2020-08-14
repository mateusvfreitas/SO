CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-join.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: join

join: pingpong-join.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) join