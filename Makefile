CFLAGS=-O0

hcc: hcc.o tokenizer.o parser.o codegen.o utils.o map.o vector.o
	gcc -o hcc hcc.o tokenizer.o parser.o codegen.o utils.o map.o vector.o

utils.o: src/utils.c
	gcc -c src/utils.c $(CFLAGS)

tokenizer.o: src/tokenizer.c
	gcc -c src/tokenizer.c $(CFLAGS)

parser.o: src/parser.c
	gcc -c src/parser.c $(CFLAGS)

codegen.o: src/codegen.c
	gcc -c src/codegen.c $(CFLAGS)

map.o: src/map.c
	gcc -c src/map.c $(CFLAGS)

vector.o: src/vector.c
	gcc -c src/vector.c $(CFLAGS)

hcc.o: src/hcc.c
	gcc -c src/hcc.c $(CFLAGS)

test.o:
	gcc -c test/test.c $(CFLAGS)

test: hcc test.o vector.o map.o
	gcc -o test.a test.o vector.o map.o
	./test.a
	./test/test.sh

clean:
	rm -f hcc test.a *.o *~ tmp* *_tmp*
