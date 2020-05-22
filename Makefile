CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

c2: $(OBJS)
	$(CC) -o c2 $(OBJS) $(LDFLAGS)

$(OBJS): c2.h

test: c2
	./test.sh

clean:
	rm -f c2 *.o *~ tmp* *.log
.PHONY: test clean
