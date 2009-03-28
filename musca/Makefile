PREFIX?=/usr/X11R6
CFLAGS?=-Wall -g

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include -L$(PREFIX)/lib -lc -lm -lX11 -o musca musca.c

clean:
	rm -f musca

