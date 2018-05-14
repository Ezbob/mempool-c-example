# mempool-c-example
Super small example of a memory pool written in C.
Memory pool can be used to enhanced the speed of memory allocation by preallocating a large pool of memory that 
then can be used to define data structures in the program. 

This memory pool uses a freelist to point to the free memory slots, and is implemented as a singular linked list.

