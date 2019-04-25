hcc:
	./src/hcc.c

test: hcc
	./test/test.sh

clean:
	rm -f hcc *.o *~ tmp*
