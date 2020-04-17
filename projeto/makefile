CC=gcc
CFLAGS=-I. -Wall -g
DEPS = datatypes.h pingpong.h queue.h
OBJ = pingpong.o pingpong-scheduler.o queue.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: scheduler

scheduler: pingpong-scheduler.o pingpong.o queue.o
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY : clean
clean :
	-rm -f $(OBJ) scheduler