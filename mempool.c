#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef char** freelist_t;

struct mempool {
    char *memspace;
    freelist_t free;
    size_t capacity;
    struct mempool *next;
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
        return NULL;
    }
    mp->next = NULL;
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

/*  test struct */
struct a {
    int a;
    long b;
} a;

int main() {
    size_t size = 100;

    int b = 20;

    /* simple long example */

    struct mempool *mp = mempool_init(sizeof(long), size);

    printf("-->%lu %lu\n", sizeof(struct freelist), sizeof(char));

    printf(":: %p\n", (void *) mp->free);

    long *a = mempool_take(mp);

    printf("in: %s\n", (mempool_hasaddr(mp, a) ? "true" : "false") );
    printf("in: %s\n", (mempool_hasaddr(mp, &b) ? "true" : "false") );

    printf(">> %p n: %p\n", (void *) a, (void *) mp->free);
    *a = 42L;

    printf("--> alloced %p %lu\n", (void *) a, *a);

    mempool_recycle(mp, a);

    printf("now free %p\n", (void *) mp->free);
    for ( size_t i = 0; i < size; ++i ) {
        long *p = mempool_take(mp);
        printf("%p\n", (void *) p);
        mempool_recycle(mp, p);
    }
    mempool_del(mp);
    struct mempool *mpa = mempool_init(sizeof(struct a), size);
    struct a *n = mempool_take(mpa);

    n->a = 10;
    n->b = 11L;

    printf("%i -> %li\n", n->a, n->b);

    mempool_recycle(mpa, n);
    
    struct a *structs[50] = {0};

    for ( size_t i = 0; i < 50; ++i ) {
        struct a *p = mempool_take(mpa);

        p->a = i - 1;
        p->b = i + 10;

        printf("%lu: %p %i %lu \n", i, (void *) p, p->a, p->b);
        structs[i] = p;
    }

    for ( size_t i = 0; i < 20; ++i ) {
        printf("%lu: %p \n", i , (void *) structs[i]);
        mempool_recycle(mpa, structs[i]);
    }

    size_t count = 0;
    for ( count = 0; mpa->free != NULL; ++count ) {
        struct a *k = mempool_take(mpa);
        printf("%lu: %p %s\n", count, (void *) k, (mempool_hasaddr(mpa, k) ? "true" : "false") );
    }

    assert(count == 70);

    mempool_del(mpa);

    return EXIT_SUCCESS;
}

