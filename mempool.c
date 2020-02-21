
#include "mempool.h"

#include <stdlib.h>

static inline int _mempool_init_impl(struct mempool *mp, size_t itemsize,
                                     size_t poolsize) {

  mp->capacity = poolsize * itemsize;
  mp->memspace = mp->allocator(mp->capacity);
  if (mp->memspace == NULL) {
    return -1;
  }

  mp->free = mp->allocator((poolsize + 1) * sizeof(unsigned char *));
  if (mp->free == NULL) {
    mp->deallocator(mp->memspace);
    return -1;
  }

  for (size_t i = 0; i < poolsize; ++i) {
    mp->free[i] = mp->memspace + (itemsize * i);
  }
  mp->free[poolsize + 1] = NULL; // sentinel
  mp->next_free = mp->free;

  return 0;
}

/**
 * mempool initializer with default allocators
 */
int mempool_init(struct mempool *mp, size_t itemsize, size_t poolsize) {

  mp->allocator = malloc;
  mp->deallocator = free;

  return _mempool_init_impl(mp, itemsize, poolsize);
}

/**
 * mempool initializer with custom allocators
 */
int mempool_init2(struct mempool *mp, size_t itemsize, size_t poolsize,
                  void *(*allocator)(size_t), void (*deallocator)(void *)) {

  mp->allocator = allocator;
  mp->deallocator = deallocator;

  return _mempool_init_impl(mp, itemsize, poolsize);
}

void mempool_deinit(struct mempool *mp) {
  printf("mem: %p, free: %p\n", mp->memspace, mp->free);
  unsigned char **it = mp->free;
  while( (*it) != NULL ) {
    *it = NULL;
    it++;
  }
  mp->deallocator(mp->memspace);
  printf("--> %p\n", *mp->free);
  mp->deallocator(&mp->free);
  mp->next_free = NULL;
  mp->memspace = NULL;
  mp->free = NULL;
  mp->capacity = 0;
}

/*
 * Takes itemsize memory from the memory pool.
 */
void *mempool_take(struct mempool *mp) {
  if ((*mp->next_free) != NULL) {
    unsigned char **fl = mp->next_free;
    mp->next_free = fl + 1;
    return (void *)(*fl);
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
  if (mp->next_free == mp->free) {
    return; // we have reached the head of the free list
  }
  unsigned char **free = mp->next_free - 1;
  *free = ((unsigned char *) mem);
  mp->next_free = free;
}
