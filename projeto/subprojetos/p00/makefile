CC=gcc
CFLAGS=-I. -Wall -g
DEPS = queue.h
OBJ = queue.o testafila.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

queue: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm -f $(OBJ) queue
