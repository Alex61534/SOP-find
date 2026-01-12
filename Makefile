CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

myfind: src/main.c src/walk.c src/filters.c
	$(CC) $(CFLAGS) src/main.c src/walk.c src/filters.c -o myfind

clean:
	rm -f myfind
