CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-maintask.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: maintask

maintask: pingpong-maintask.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) maintask