test: spscq.o test/test.c
	gcc test/test.c spscq.o -I src/ -o test.out -g -Wall && ./test.out

spscq.o: src/spscq.c src/spscq.h
	gcc -c $< -I src/ -g

clean:
	rm -rf *.o *.out
