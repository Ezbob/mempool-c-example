# mempool-c-example
Super small example of a mempool written in C.
Mempool can be used to enhanced the speed of memory allocation by preallocating a pool of memory that then can be used to 
define data structures in the program. 

This mempool uses a freelist to point to the free memory slots, and is implemented as a singular linked list.

