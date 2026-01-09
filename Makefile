CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

myfind: src/main.c src/walk.c
	$(CC) $(CFLAGS) src/main.c src/walk.c -o myfind

clean:
	rm -f myfind

