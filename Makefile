CFLAGS=-std=gnu11 -Wall -Wextra -O2 -g

all: autoclick
autoclick: autoclick.c
clean:
	rm -f *.o autoclick
.PHONY: all clean
