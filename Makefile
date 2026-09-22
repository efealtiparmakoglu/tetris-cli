CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra -Wpedantic

all: tetris-cli

tetris-cli: src/main.c src/tetris.c src/tetris.h
	$(CC) $(CFLAGS) -Isrc -o $@ src/main.c src/tetris.c

tetris_tests: tests/tetris_tests.c src/tetris.c src/tetris.h
	$(CC) $(CFLAGS) -Isrc -o tetris_tests tests/tetris_tests.c src/tetris.c

test: tetris_tests tetris-cli
	./tetris_tests
	./tetris-cli score /tmp/tetris-no-scores.txt | grep -q empty && echo "score OK"

clean:
	rm -f tetris-cli tetris_tests
