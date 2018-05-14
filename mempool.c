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

struct freelist *freelist_init(char *mspace) {
    struct freelist *n = malloc(sizeof(struct freelist));
    n->mem = mspace;
    n->next = NULL;
    return n;
}

struct freelist *freelist_del(struct freelist *fl) {
    fl->mem = NULL;
    struct freelist *n = fl->next;
    free(fl);
    return n;
}

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

void mempool_del(struct mempool *mp) {
    struct freelist *iter = mp->free;
    while ( (iter = freelist_del(iter)) != NULL );
    free(mp->memspace);
    free(mp);
}

void *mempool_alloc(struct mempool *mp) {
    if ( mp->free != NULL ) {
        struct freelist *fl = mp->free;
        void *res = (void *) fl->mem;
        mp->free = freelist_del(fl);
        return res;
    } else {
        return NULL;
    }
}

void mempool_free(struct mempool *mp, void *mem) {
    struct freelist *n = freelist_init((char *) mem);
    n->next = mp->free;
    mp->free = n;
}


int main() {
    struct mempool *mp = mempool_init(sizeof(int), 1000);

    printf("so many items %lu\n", mp->capacity);

    int *a = mempool_alloc(mp);
    *a = 42;

    printf("--> alloced %i\n", *a);

    mempool_free(mp, a);

    mempool_del(mp);

    return EXIT_SUCCESS;
}

