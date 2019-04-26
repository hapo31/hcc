hcc: hcc.o vector.o
	gcc -o hcc hcc.o vector.o

vector.o: src/vector.c
	gcc -c src/vector.c

hcc.o: src/hcc.c
	gcc -c src/hcc.c

test.o: vector.o
	gcc -c test/test.c

test: hcc test.o vector.o
	./test/test.sh
	gcc -o test.a test.o vector.o
	./test.a

clean:
	rm -f hcc test.a *.o *~ tmp*
