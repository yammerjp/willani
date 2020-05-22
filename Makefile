CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): willani.h

test: willani
	./test.sh

clean:
	rm -f willani *.o *~ tmp* *.log
.PHONY: test clean
