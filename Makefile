
LDFLAGS = -lpthread
CFLAGS = -Wall -Wextra

CC = gcc $(CFLAGS)

OBJS = utils.o map.o
BINS = seq

all: $(OBJS) $(BINS)

seq: seq.o
	$(CC) $(OBJS) seq.o -o seq

seq.o: seq.c
utils.o: utils.c utils.h
map.o: map.c map.h

clean:
	rm -f $(BINS) *.o
