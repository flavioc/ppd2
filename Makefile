
LDFLAGS = -lpthread
DEBUG = -g3
WARNINGS = -Wall -Wextra
OPTMS = -O2
CFLAGS = $(WARNINGS) $(DEBUG) $(OPTMS)

OBJS = utils.o map.o object.o position.o thread.o
BINS = seq simulator generator simulator2

all: $(OBJS) $(BINS)

seq: seq.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) seq.o -o seq

simulator: simulator.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) simulator.o -o simulator -lpthread

simulator2: simulator2.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) simulator2.o -o simulator2 -lpthread

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
