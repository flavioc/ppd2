
LDFLAGS = -lpthread
CFLAGS = -Wall -Wextra -g3 -O2

OBJS = utils.o map.o object.o position.o thread.o
BINS = seq simulator generator

all: $(OBJS) $(BINS)

seq: seq.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) seq.o -o seq

simulator: simulator.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) simulator.o -o simulator -lpthread

generator: generator.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) generator.o -o generator

seq.o: seq.c
simulator.o: simulator.c
utils.o: utils.c utils.h
map.o: map.c map.h
object.o: object.c object.h
position.o: position.c position.h
thread.o: thread.c thread.h

clean:
	rm -f $(BINS) *.o
