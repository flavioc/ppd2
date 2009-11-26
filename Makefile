
LDFLAGS = -lpthread
CFLAGS = -Wall -Wextra

OBJS = utils.o map.o object.o position.o
BINS = seq

all: $(OBJS) $(BINS)

seq: seq.o $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) seq.o -o seq

seq.o: seq.c
utils.o: utils.c utils.h
map.o: map.c map.h
object.o: object.c object.h
position.o: position.c position.h

clean:
	rm -f $(BINS) *.o
