#include "mempool.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

  printf("Pointer size --> %llu\n", sizeof(char **));

  printf("free list pointer --> %p\n", (void *)mp->free);

  void *initial_free = (void *)mp->free;

  long *a = mempool_take(mp);

  printf("a (taken from mempool) is in mempool: %s\n",
         (mempool_hasaddr(mp, a) ? "true" : "false"));
  assert(mempool_hasaddr(mp, a));
  printf("b (not taken from  in mempool: %s\n",
         (mempool_hasaddr(mp, &b) ? "true" : "false"));
  assert(!mempool_hasaddr(mp, &b));

  assert(initial_free != (void *)mp->free);
  *a = 42L;

  mempool_recycle(mp, a);

  assert(initial_free == (void *)mp->free);

  for (size_t i = 0; i < size; ++i) {
    long *p = mempool_take(mp);
    assert(mempool_hasaddr(mp, p));
  }
  mempool_del(mp);

  printf(
      "testing struct pointers allocated and deallocated from the mempool\n");
  struct mempool *mpa = mempool_init(sizeof(struct a), size);
  struct a *n = mempool_take(mpa);

  n->a = 10;
  n->b = 11L;

  printf("setting struct (a: %i, b: %li)\n", n->a, n->b);
  assert(mempool_hasaddr(mpa, n));

  mempool_recycle(mpa, n);

  struct a *structs[50] = {0};
  char **freepointers[50] = {0};

  for (size_t i = 0; i < 50; ++i) {
    freepointers[i] = mpa->free;
    struct a *p = mempool_take(mpa);

    p->a = i - 1;
    p->b = i + 10;

    assert(mempool_hasaddr(mpa, p));
    printf("struct %llu: (addr: %p, a: %i, b: %lu)\n", i, (void *)p, p->a,
           p->b);
    structs[i] = p;
  }

  for (size_t i = 0; i < 50; ++i) {
    struct a *p = structs[i];
    printf("struct %llu: (addr: %p, a: %i, b: %lu)\n", i, (void *)p, p->a,
           p->b);

    assert(mempool_hasaddr(mpa, p));
    assert(p->a == ((int)i) - 1);
    assert(p->b == ((long)i) + 10);
    char **free = freepointers[49 - i];
    printf("%p = %p\n", (void *)free, (void *)mpa->free);
    // assert(free == mpa->free);
    mempool_recycle(mpa, p);
  }

  mempool_del(mpa);

  return EXIT_SUCCESS;
}
