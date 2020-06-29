CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c) $(wildcard src/parse/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h src/parse/parse.h

test: willani
	rm -f test.s test.out
	echo 'int static_fn() { return 5; }' | gcc -xc -c -o tmp2.o -
	./willani test/test.c > test.s
	gcc -static test.s tmp2.o -o test.out
	./test.out
	@echo 'test1 is finished!!'
	./test/test2.sh
	@echo 'test2 is finished!!'

sample: willani
	./willani sample.c > tmp.s
	gcc -static tmp.s -o tmp
	-./tmp

debug: willani
	./willani sample.c > tmp.s
	gcc -g -static tmp.s -o tmp
	gdb tmp

clean:
	rm -f willani src/*.o src/parse/*.o *~ tmp* *.log core test.s test.out test10.c test10.s test11.c test11.s test10.out
.PHONY: test clean sample debug
