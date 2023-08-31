// a few allocations in multiples of 4 bytes
#include <assert.h>
#include <stdlib.h>
#include "p4Heap.h"
#include <stdio.h>

int main() {
   assert(init_heap(4096) == 0);
   void* ptr1 = balloc(4);
   printf("%08x", (unsigned int)(ptr1));
   //disp_heap();
   void* ptr2 = balloc(8);
   //disp_heap();
   printf("%08x", (unsigned int)(ptr2));
   assert(ptr2 != NULL);
   void* ptr3 = balloc(16);
   printf("%08x", (unsigned int)(ptr3));
   //disp_heap();
   assert(ptr3 != NULL);
   void* ptr4 = balloc(24);
   printf("%08x", (unsigned int)(ptr4));
   assert(ptr4 != NULL);
   printf("heap :");
   disp_heap();
   exit(0);
}
