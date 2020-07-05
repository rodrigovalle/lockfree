test: lib.o
	gcc test.c lib.o -o test.out -g -Wall && ./test.out

lib.o: lib.c lib.h
	gcc -c lib.c -g

clean:
	rm -rf *.o *.out
