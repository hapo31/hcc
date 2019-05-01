hcc: hcc.o vector.o map.o
	gcc -o hcc hcc.o vector.o map.o

utils.o: src/utils.c
	gcc -c src/utils.c

map.o: src/map.c
	gcc -c src/map.c

vector.o: src/vector.c
	gcc -c src/vector.c

hcc.o: src/hcc.c
	gcc -c src/hcc.c

test.o: vector.o map.o
	gcc -c test/test.c

test: hcc test.o vector.o map.o
	gcc -o test.a test.o vector.o map.o
	./test.a
	./test/test.sh

clean:
	rm -f hcc test.a *.o *~ tmp*
