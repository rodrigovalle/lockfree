.PHONY: clean lint

test: spscq.o test/test.c
	gcc test/test.c spscq.o -I src/ -o test.out -g -Wall && ./test.out

spscq.o: src/spscq.c src/spscq.h
	gcc -c $< -I src/ -g

# I think this command might be covered the clang-analyzer checks
#   clang-check src/*.c test/*.c -- -I src/
lint:
	clang-tidy src/*.c test/*.c --checks='clang-analyzer-*,performance-*,ccpcoreguidelines-*,bugprone-*' -- -I src/
	cppcheck src/*.c test/*.c --language=c --enable=style -I src/

clean:
	rm -rf *.o *.out *.plist
