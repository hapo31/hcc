CFLAGS=-O0 -g
SRCS=$(wildcard src/*.c)
OBJS=$(notdir $(SRCS:%.c=%.o))

hcc: $(OBJS)
	gcc -o hcc $(OBJS)

$(OBJS): $(SRCS)
	gcc -c $(SRCS) $(CFLAGS)

test.o:
	gcc -c test/test.c $(CFLAGS)

test: hcc ./test/test.sh test.o vector.o map.o
	gcc -o test.a test.o vector.o map.o
	./test.a
	./test/test.sh

clean:
	rm -f hcc test.a *.o *~ tmp* *_tmp*
