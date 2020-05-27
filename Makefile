CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h

test: willani
	bin/test.sh

sample: willani
	bin/run-sample.sh

debug: willani
	bin/gdb-sample.sh

clean:
	rm -f willani src/*.o *~ tmp* *.log core
.PHONY: test clean sample debug
