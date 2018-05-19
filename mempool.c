#include <stdlib.h>
#include <stdio.h>

struct freelist {
    struct freelist *next;
} freelist; 

struct mempool {
    size_t itemsize;
    size_t capacity;
    size_t freesize;
    char *memspace;
    struct freelist *free;
};


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
    mp->memspace = malloc(cap * (itemsize + sizeof(struct freelist)));
                    // strip linked list elements in the memspace
    if ( mp->memspace == NULL ) {
        return NULL;
    }
    size_t blocksize = itemsize + sizeof(struct freelist);

    printf(">>> %lu %lu %lu %lu %lu\n", cap * blocksize, cap, blocksize, sizeof(struct freelist), itemsize);
    mp->free = (struct freelist *) mp->memspace;

    struct freelist *iter = mp->free;
    for ( size_t i = 1; i < cap; ++i ) {
        iter->next = (void *) (mp->memspace + i * blocksize);
        printf("%p - %p\n", (void *) iter, (void *) iter->next);
        iter = iter->next;
    }
    iter->next = NULL;
    mp->freesize = cap;

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
        struct freelist *fl = mp->free;
        void *res = (void *) (fl + 1); // actual memory is next to free struct
        mp->free = fl->next;
        mp->freesize--;
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
    struct freelist *memspace = mem;
    struct freelist *header = (memspace - 1);
        /* because we have casted memspace to freelist we offset by one freelist pointer */
    struct freelist *next = mp->free;
    header->next = next;
    mp->free = header;
    mp->freesize++;
    return 1;
}

/*
 * Enlarges the memory pool by adding another 'addition' items to the pool.
 */
int mempool_grow(struct mempool *mp, size_t addition) {
    (void) mp;
    (void) addition;
/*
    size_t cap = mp->capacity;
    size_t itemsize = mp->itemsize;
    size_t totalsize = (addition + cap) * itemsize;

    char *resized = realloc(mp->memspace, totalsize);
    if ( resized ) {
        mp->memspace = resized;
        for ( size_t i = cap; i < (cap + addition); ++i ) {
            if ( !mempool_recycle(mp, mp->memspace + (i * itemsize)) ) {
                return 0;
            }
        }
        mp->capacity = (cap + addition);
        return 1;
    } else {
        return 0;
    }
*/
    return 0;
}


/*  test struct */
struct a {
    int a;
    long b;
} a;

int main() {
    size_t size = 100;

    /* simple long example */

    struct mempool *mp = mempool_init(sizeof(long), size);

    printf("-->%lu %lu\n", sizeof(struct freelist), sizeof(char));

    printf(":: %p\n", (void *) mp->free);

    long *a = mempool_take(mp);
    *a = 42L;

    printf("--> alloced %p %lu\n", (void *) a, *a);

    mempool_recycle(mp, a);

    printf("now free %p\n", (void *) mp->free);

    struct freelist *fp = (struct freelist *) mp->free;
    for ( size_t i = 0; i < size; ++i ) {
        printf("%lu: %p %p \n", i, (void *) fp, (void *) fp->next);
        long *p = mempool_take(mp);
        mempool_recycle(mp, p);
        fp = fp->next;
    }

    mempool_del(mp);

    /* use of structs instead of ints */
    struct mempool *mpa = mempool_init(sizeof(struct a), size);

    struct a *n = mempool_take(mpa);

    n->a = 10;
    n->b = 11L;

    printf("%i -> %li\n", n->a, n->b);

    mempool_recycle(mpa, n);

    for ( size_t i = 0; i < size; ++i ) {
        printf("%lu: %p \n", i, (void *) mpa->free);
        struct a *p = mempool_take(mpa);
        mempool_recycle(mpa, p);
    }

    mempool_del(mpa);

    /* grow example */
/*
    struct mempool *mpb = mempool_init(sizeof(struct a), size);

    mempool_grow(mpb, 100);

    for ( size_t i = 0; i < size + 100; ++i ) {
        printf("%lu: %p \n", i, (void *) mpb->free);
        struct a *p = mempool_take(mpb);
        mempool_recycle(mpb, p);
    }

    mempool_del(mpb);
*/
    return EXIT_SUCCESS;
}

