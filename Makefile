CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c) $(wildcard src/parse/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h src/parse/parse.h

test: willani
	bin/test.sh

sample: willani
	bin/run-sample.sh

debug: willani
	bin/gdb-sample.sh

clean:
	rm -f willani src/*.o src/parse/*.o *~ tmp* *.log core test.c
.PHONY: test clean sample debug
