CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h

test: willani
	bin/test.sh

clean:
	rm -f willani src/*.o *~ tmp* *.log core
.PHONY: test clean
