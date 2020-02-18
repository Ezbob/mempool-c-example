
#include "mempool.h"

#include <stdlib.h>

/**
 * mempool initializer with default allocators
 */
int mempool_init(struct mempool *mp, size_t itemsize, size_t poolsize) {

  mp->allocator = malloc;
  mp->deallocator = free;

  size_t blocksize = itemsize + sizeof(unsigned char *);
  mp->capacity = poolsize * blocksize;
  mp->memspace = mp->allocator(mp->capacity);
  if (mp->memspace == NULL) {
    return -1;
  }
  mp->free = (unsigned char **)(mp->memspace);

  unsigned char **iter = mp->free;
  for (size_t i = 1; i < poolsize; ++i) {
    void *next = (mp->memspace + i * blocksize);
    *iter = next; // make current free pointer point to start of next block
    iter = (unsigned char **)next; // advance to next free pointer
  }
  *iter = NULL;

  return 0;
}

/**
 * mempool initializer with custom allocators
 */
int mempool_init2(struct mempool *mp, size_t itemsize, size_t poolsize,
                  void *(*allocator)(size_t), void (*deallocator)(void *)) {

  mp->allocator = allocator;
  mp->deallocator = deallocator;

  size_t blocksize = itemsize + sizeof(unsigned char *);
  mp->capacity = poolsize * blocksize;
  mp->memspace = allocator(mp->capacity);
  if (mp->memspace == NULL) {
    return -1;
  }
  mp->free = (unsigned char **)(mp->memspace);

  unsigned char **iter = mp->free;
  for (size_t i = 1; i < poolsize; ++i) {
    void *next = (mp->memspace + i * blocksize);
    *iter = next; // make current free pointer point to start of next block
    iter = (unsigned char **)next; // advance to next free pointer
  }
  *iter = NULL;

  return 0;
}

void mempool_deinit(struct mempool *mp) {
  mp->deallocator(mp->memspace);
  mp->memspace = NULL;
  mp->free = NULL;
  mp->capacity = 0;
}

/*
 * Takes itemsize memory from the memory pool.
 */
void *mempool_take(struct mempool *mp) {
  if (mp->free != NULL) {
    unsigned char **fl = mp->free;
    void *res = (void *)(fl + 1); // actual memory is next to free pointer
    mp->free = (void *)*fl;
    return res;
  } else {
    return NULL;
  }
}

/*
 * Check if memory is in pool
 */
int mempool_hasaddr(struct mempool *mp, void *mem) {
  return mp->memspace <= ((unsigned char *)mem) &&
         (mp->memspace + mp->capacity) > ((unsigned char *)mem);
}

/*
 * Puts itemsize size memory back into the pool,
 * by putting it on the freelist.
 */
void mempool_recycle(struct mempool *mp, void *mem) {
  unsigned char **header = (((unsigned char **)mem) -
                            1); // next free pointer is to the left of the data
  *header = (void *)mp->free;
  mp->free = header;
}
