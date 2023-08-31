// a simple 8 byte allocation
#include <assert.h>
#include <stdlib.h>
#include "p4Heap.h"
#include <stdio.h>

int main() {
    assert(init_heap(4096) == 0);
    void* ptr = balloc(8);
    printf("%08x", (unsigned int)(ptr));
    disp_heap();
    assert(ptr != NULL);
    exit(0);
}
