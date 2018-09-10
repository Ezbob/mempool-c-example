#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdlib.h>

/*
 * Mempool struct
 */
struct mempool {
    char *memspace; /* memory space */
    char **free; /* next free pointer into memspace */
    size_t capacity; /* byte capacity of mempool */
};


/*
 * Constructor for the mempool.
 * The 'itemsize' size parameter is the size in bytes of the entities
 * that the mempool represents a pool of; and the 'cap' parameter is
 * the maximum number of such items that the pool is enable to supply
 * any consumers with.
 */
struct mempool *mempool_init(size_t itemsize, size_t poolsize) {
    struct mempool *mp = malloc(sizeof(struct mempool));
    if ( mp == NULL ) {
        return NULL;
    }
    size_t blocksize = itemsize + sizeof(char *);
    mp->capacity = poolsize * blocksize;
    mp->memspace = malloc(mp->capacity);
    if ( mp->memspace == NULL ) {
        free(mp);
        return NULL;
    }
    mp->free = (char **) (mp->memspace);

    char **iter = mp->free;
    for ( size_t i = 1; i < poolsize; ++i ) {
        void *next = (mp->memspace + i * blocksize);
        *iter = next; // make current free pointer point to start of next block
        iter = (char **) next; // advance to next free pointer
    }
    *iter = NULL;

    return mp;
}

/*
 * Memory pool destructor.
 * Deallocates the remaining records in the free lists, and deallocates
 * the heap memory allocate to represent the memory pool.
 */
void mempool_del(struct mempool *mp) {
    free(mp->memspace);
    free(mp);
}

/*
 * Takes itemsize memory from the memory pool.
 */
void *mempool_take(struct mempool *mp) {
    if ( mp->free != NULL ) {
        char **fl = mp->free;
        void *res = (void *) (fl + 1); // actual memory is next to free pointer
        mp->free = (void *) *fl;
        return res;
    } else {
        return NULL;
    }
}

/*
 * Check if memory is in pool 
 */
int mempool_hasaddr(struct mempool *mp, void *mem) {
    return mp->memspace <= ((char *) mem) && (mp->memspace + mp->capacity) > ((char *) mem);
}

/*
 * Puts itemsize size memory back into the pool,
 * by putting it on the freelist.
 */
void mempool_recycle(struct mempool *mp, void *mem) {
    char **header = (((char **) mem) - 1); // next free pointer is to the left of the data
    *header = (void *) mp->free;
    mp->free = header;
}

#endif



