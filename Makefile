CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c) $(wildcard src/parse/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h src/parse/parse.h

test: willani
	rm -f test.s test.out
	./willani test/test.c > test.s
	gcc -static test.s -o test.out
	./test.out

sample: willani
	bin/run-sample.sh

debug: willani
	bin/gdb-sample.sh

clean:
	rm -f willani src/*.o src/parse/*.o *~ tmp* *.log core test.s test.out
.PHONY: test clean sample debug
