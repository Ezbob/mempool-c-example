
CFLAGS=-std=c99 -g -Wall -Wextra -pedantic

mempool: mempool.c
	$(CC) $(CFLAGS) $^ -o $@

