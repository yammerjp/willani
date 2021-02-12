CFLAGS=-std=c11 -g -static
SRCS=$(wildcard src/*.c) $(wildcard src/parse/*.c)
OBJS=$(SRCS:.c=.o)

willani: $(OBJS)
	$(CC) -o willani $(OBJS) $(LDFLAGS)

$(OBJS): src/willani.h src/parse/parse.h

test: willani
	rm -f test-*
	echo 'int static_fn() { return 5; }' | gcc -xc -c -o test-01.o -
	./willani test/test.c -o test-00.s
	gcc -static test-00.s test-01.o -o test-00.out
	./test-00.out
	@echo 'test1 is finished!!'
	./test/test2.sh
	@echo 'test2 is finished!!'
	./test/test3.sh
	@echo 'test3 is finished!!'

sample: willani
	./willani sample.c -o tmp.s
	gcc -static tmp.s -o tmp
	-./tmp

debug: willani
	./willani sample.c -o tmp.s
	gcc -g -static tmp.s -o tmp
	gdb tmp

clean:
	rm -rf willani* src/*.o src/*.s src/parse/*.o src/parse/*.s *~ tmp* *.log core test-* asm* obj src_self ast.json token.json
.PHONY: test clean sample debug
