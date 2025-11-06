CFLAGS=-std=gnu11 -Wall -Wextra -O2 -g

all: autoclick

autoclick.o: autoclick.c config.h
autoclick: autoclick.o

clean:
	rm -f *.o autoclick
.PHONY: all clean
