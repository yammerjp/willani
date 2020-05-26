CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h

test: willani
	bin/test.sh

sample: willani
	bin/run-samplecode.sh

debug: willani
	bin/gdb-samplecode.sh

clean:
	rm -f willani src/*.o *~ tmp* *.log core
.PHONY: test clean sample debug
