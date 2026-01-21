CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -D_POSIX_C_SOURCE=200809L -pthread

myfind: src/main.c src/walk.c src/filters.c src/queue.c src/cli.c src/io.c
	$(CC) $(CFLAGS) src/main.c src/walk.c src/filters.c src/queue.c src/cli.c src/io.c -o myfind

clean:
	rm -f myfind

.PHONY: test
test: myfind
	./tests/run.sh
