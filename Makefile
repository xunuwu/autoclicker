CFLAGS=-std=gnu11 -Wall -Wextra -O2

all: autoclick
autoclick:
clean:
	rm -f *.o autoclick
.PHONY: all clean
