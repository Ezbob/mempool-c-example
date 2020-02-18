/**
 * mempool.c - A super simple implementation of a memory pool.
 * This implementation uses a fixed-size mempool of fixed-size records.
 *
 * Some terminology used in the comments:
 * - record: a piece of user-data that needs to be stored in the pool
 * - block: a piece of data that is a record plus a embedded free-pointer
 * 
 * Author: Anders Busch (2018-2020)
 */

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdint.h>

/*
 * Mempool struct
 */
struct mempool
{
    void *(*allocator)(size_t);   /* allocator, interface is the same as stdlib malloc */
    void (*deallocator)(void *);  /* deallocator, interface is the same as stdlib free */

    unsigned char *memspace;      /* memory space */
    unsigned char **free;         /* next free pointer into memspace */
    size_t capacity;              /* byte capacity of mempool */
};

/*
 * Memory pool initializer.
 * The 'itemsize' parameter represent the size of a record in the pool.
 * 'poolsize' represents number of records that should be contained in the memory pool.
 * 
 * Returns 0 on success, -1 on error.
 */
int mempool_init(struct mempool *, size_t itemsize, size_t poolsize);

/*
 * Memory pool initializer.
 * This gives possibility of using custom deallocator/allocator.
 * The allocator and deallocator has the same function signature as stdlib malloc/free.
 * 
 * Returns 0 on success, -1 on error.
 */
int mempool_init2(struct mempool *mp, size_t itemsize, size_t poolsize,
                  void *(*allocator)(size_t), void (*deallocator)(void *));

/*
 * Memory pool de-initializer.
 * Deallocates the remaining records in the free lists, and memory pool
 * fields.
 */
void mempool_deinit(struct mempool *mp);

/*
 * Takes a record from the memory pool.
 */
void *mempool_take(struct mempool *mp);

/*
 * Check if record is in pool
 */
int mempool_hasaddr(struct mempool *mp, void *mem);

/*
 * Puts record back into the pool, making it available to
 * others again.
 */
void mempool_recycle(struct mempool *mp, void *mem);

#endif
