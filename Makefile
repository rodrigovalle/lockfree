.PHONY: clean lint
CC=gcc
INCLUDE=-I src/
CFLAGS=-g -Wall

# Makefile cheatsheet:
# $^: The names of all the prerequisites, separated by spaces.
# $<: The name of the first prerequisite.
# $@: The filename of the target of the rule.

test: unittest.out
	./unittest.out

benchmark: benchmark.out
	./benchmark.out

unittest.out: spscq.o test/unittest.c
	$(CC) $(CFLAGS) $^ $(INCLUDE) -o $@

benchmark.out: spscq.o test/benchmark.c
	$(CC) $(CFLAGS) -pthread $^ $(INCLUDE) -o $@

spscq.o: src/spscq.c src/spscq.h
	$(CC) $(CFLAGS) -c $< $(INCLUDE) -o $@

# I think this command might be covered the clang-analyzer checks
#   clang-check src/*.c test/*.c -- -I src/
lint:
	clang-tidy src/*.c test/*.c --checks='clang-analyzer-*,performance-*,ccpcoreguidelines-*,bugprone-*' -- $(INCLUDE)
	cppcheck src/*.c test/*.c --language=c --enable=style $(INCLUDE)

clean:
	rm -rf *.o *.out *.plist
