#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdlib.h>

/*
 * Mempool struct
 */
struct mempool
{
    char *memspace;  /* memory space */
    char **free;     /* next free pointer into memspace */
    size_t capacity; /* byte capacity of mempool */
};

/*
 * Constructor for the mempool.
 * The 'itemsize' size parameter is the size in bytes of the entities
 * that the mempool represents a pool of; and the 'cap' parameter is
 * the maximum number of such items that the pool is enable to supply
 * any consumers with.
 */
struct mempool *mempool_init(size_t itemsize, size_t poolsize);

/*
 * Memory pool destructor.
 * Deallocates the remaining records in the free lists, and deallocates
 * the heap memory allocate to represent the memory pool.
 */
void mempool_del(struct mempool *mp);

/*
 * Takes itemsize memory from the memory pool.
 */
void *mempool_take(struct mempool *mp);

/*
 * Check if memory is in pool 
 */
int mempool_hasaddr(struct mempool *mp, void *mem);

/*
 * Puts itemsize size memory back into the pool,
 * by putting it on the freelist.
 */
void mempool_recycle(struct mempool *mp, void *mem);

#endif
