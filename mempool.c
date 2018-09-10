#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct mempool {
    char *memspace;
    char **free;
    size_t capacity;
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

/*  test struct */
struct a {
    int a;
    long b;
} a;

int main() {
    size_t size = 100;

    int b = 20;

    /* simple long example */

    printf("testing long pointers allocated and deallocated from the mempool\n");

    struct mempool *mp = mempool_init(sizeof(long), size);

    printf("Pointer size --> %lu\n", sizeof(char **));

    printf("free list pointer --> %p\n", (void *) mp->free);

    void *initial_free = (void *) mp->free;

    long *a = mempool_take(mp);

    printf("a (taken from mempool) is in mempool: %s\n", (mempool_hasaddr(mp, a) ? "true" : "false") );
    assert(mempool_hasaddr(mp, a));
    printf("b (not taken from  in mempool: %s\n", (mempool_hasaddr(mp, &b) ? "true" : "false") );
    assert(!mempool_hasaddr(mp, &b));

    assert(initial_free != (void *) mp->free);
    *a = 42L;

    mempool_recycle(mp, a);

    assert(initial_free == (void *) mp->free);

    for ( size_t i = 0; i < size; ++i ) {
        long *p = mempool_take(mp);
        assert(mempool_hasaddr(mp, p));
    }
    mempool_del(mp);

    printf("testing struct pointers allocated and deallocated from the mempool\n");
    struct mempool *mpa = mempool_init(sizeof(struct a), size);
    struct a *n = mempool_take(mpa);

    n->a = 10;
    n->b = 11L;

    printf("setting struct (a: %i, b: %li)\n", n->a, n->b);
    assert(mempool_hasaddr(mpa, n));

    mempool_recycle(mpa, n);
    
    struct a *structs[50] = {0};
    char **freepointers[50] = {0};

    for ( size_t i = 0; i < 50; ++i ) {
        freepointers[i] = mpa->free;
        struct a *p = mempool_take(mpa);

        p->a = i - 1;
        p->b = i + 10;
        
        assert(mempool_hasaddr(mpa, p));
        printf("struct %lu: (addr: %p, a: %i, b: %lu)\n", i, (void *) p, p->a, p->b);
        structs[i] = p;
    }

    for ( size_t i = 0; i < 50; ++i ) {
        struct a *p = structs[i];
        printf("struct %lu: (addr: %p, a: %i, b: %lu)\n", i, (void *) p, p->a, p->b);

        assert(mempool_hasaddr(mpa, p));
        assert(p->a == ((int) i) - 1);
        assert(p->b == ((long) i) + 10);
        char **free = freepointers[49 - i];
        printf("%p = %p\n", (void *) free, (void *) mpa->free);
        //assert(free == mpa->free);
        mempool_recycle(mpa, p);
    }

    mempool_del(mpa);

    return EXIT_SUCCESS;
}

