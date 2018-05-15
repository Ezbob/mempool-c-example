#include <stdlib.h>
#include <stdio.h>

struct freelist {
    char *mem;
    struct freelist *next;
} freelist; 

struct mempool {
    size_t itemsize;
    size_t capacity;
    char *memspace;
    struct freelist *free;
};

/*
 * freelist constructor. Uses the 'mspace' parameter as a
 * pointer to a record in the memory pool.
 */
struct freelist *freelist_init(char *mspace) {
    struct freelist *n = malloc(sizeof(struct freelist));
    n->mem = mspace;
    n->next = NULL;
    return n;
}

/*
 * freelist destructor. Does NOT free any memory pointed
 * from it.
 */
struct freelist *freelist_del(struct freelist *fl) {
    fl->mem = NULL;
    struct freelist *n = fl->next;
    free(fl);
    return n;
}

/*
 * Constructor for the mempool.
 * The 'itemsize' size parameter is the size in bytes of the entities
 * that the mempool represents a pool of; and the 'cap' parameter is
 * the maximum number of such items that the pool is enable to supply
 * any consumers with.
 */
struct mempool *mempool_init(size_t itemsize, size_t cap) {
    struct mempool *mp = malloc(sizeof(struct mempool));
    mp->itemsize = itemsize;
    mp->capacity = cap;
    mp->memspace = malloc(cap);
    if ( mp->memspace == NULL ) {
        return NULL;
    }
    mp->free = freelist_init(mp->memspace);

    struct freelist *iter = mp->free;
    for ( size_t i = 0; i < cap; ++i ) {
        if ( i < cap - 1 ) {
            iter->next = freelist_init(mp->memspace + i);
            iter = iter->next;
        } else {
            iter->next = NULL;
        }
    }
    return mp;
}

/*
 * Memory pool destructor.
 * Deallocates the remaining records in the free lists, and deallocates
 * the heap memory allocate to represent the memory pool.
 */
void mempool_del(struct mempool *mp) {
    struct freelist *iter = mp->free;
    while ( (iter = freelist_del(iter)) != NULL );
    free(mp->memspace);
    free(mp);
}

/*
 * Takes itemsize memory from the memory pool.
 */
void *mempool_take(struct mempool *mp) {
    if ( mp->free != NULL ) {
        struct freelist *fl = mp->free;
        void *res = (void *) fl->mem;
        mp->free = freelist_del(fl);
        return res;
    } else {
        return NULL;
    }
}

/*
 * Puts itemsize size memory back into the pool,
 * by putting it on the freelist.
 */
int mempool_recycle(struct mempool *mp, void *mem) {
    struct freelist *n = freelist_init((char *) mem);
    if ( n != NULL ) {
        n->next = mp->free;
        mp->free = n;
        return 1;
    }
    return 0;
}


int main() {
    size_t size = 100;
    struct mempool *mp = mempool_init(sizeof(int), 100);

    printf("so many items %lu\n", mp->capacity);

    int *a = mempool_take(mp);
    *a = 42;

    printf("--> alloced %i\n", *a);

    mempool_recycle(mp, a);


    for ( size_t i = 0; i < size - 1; ++i ) {
        printf(">>%lu\n", i);
        int *a = mempool_take(mp);
    }

    mempool_del(mp);

    return EXIT_SUCCESS;
}

