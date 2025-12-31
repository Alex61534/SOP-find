CC = gcc
CFLAGS = -Wall -Wextra -std=c11

myfind: src/main.c src/walk.c
	$(CC) $(CFLAGS) src/main.c src/walk.c -o myfind

clean:
	rm -f myfind

