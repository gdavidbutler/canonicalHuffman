CFLAGS = -I. -Os -g

all: huf.o encode decode

clean:
	rm -f huf.o encode decode COPYING.huf COPYING.tst

huf.o: huf.c huf.h
	$(CC) $(CFLAGS) -c huf.c

encode: test/encode.c huf.h huf.o
	$(CC) $(CFLAGS) -o encode test/encode.c huf.o

decode: test/decode.c huf.h huf.o
	$(CC) $(CFLAGS) -o decode test/decode.c huf.o

check: encode decode
	./encode COPYING > COPYING.huf
	./decode COPYING.huf > COPYING.tst
	cmp COPYING COPYING.tst
