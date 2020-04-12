CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h
OBJ = pingpong-tasks1.o pingpong-tasks2.o pingpong-tasks3.o pingpong.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: pingpong-tasks1 pingpong-tasks2 pingpong-tasks3

pingpong-tasks1: pingpong-tasks1.o pingpong.o
	$(CC) -o $@ $^ $(CFLAGS)

pingpong-tasks2: pingpong-tasks2.o pingpong.o
	$(CC) -o $@ $^ $(CFLAGS)

pingpong-tasks3: pingpong-tasks3.o pingpong.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) pingpong-tasks1 pingpong-tasks2 pingpong-tasks3
