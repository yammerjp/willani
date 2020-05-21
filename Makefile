CFLAGS=-std=c11 -g -static

c2:  c2.c

test: c2
	./test.sh
clean:
	rm -f c2 *.o *~ tmp*
.PHONY: test clean
