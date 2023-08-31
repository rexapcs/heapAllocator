
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Deb Deppeler
// Posting or sharing this file is prohibited, including any changes/additions.
//
// We have provided comments and structure for this program to help you get 
// started.  Later programs will not provide the same level of commenting,
// rather you will be expected to add same level of comments to your work.
// 09/20/2021 Revised to free memory allocated in get_board_size function.
// 01/24/2022 Revised to use pointers for CLAs
//
////////////////////////////////////////////////////////////////////////////////
// Main File:        check_board.c
// This File:        check_board.c
// Other Files:      NA
// Semester:         CS 354 Summer 2023
// Instructor:       James Sorenson
//
// Author:           Rex Hart
// Email:            rshart@wisc.edu
// CS Login:         rhart
// GG#:              (your Canvas GG number)
//                   (See https://canvas.wisc.edu/groups for your GG number)
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   including family, friencs, classmates, tutors,
//                   Peer Mentors, TAs, and Instructor.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   Avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p4Heap.h"
 
#include <stdint.h>
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE NAME or TYPE. 
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;


/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 1 
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void *balloc(int size) {
        
    int used = 0;
    if (size < 1) {
        return NULL;
    }

    int blockSize = ((size + sizeof(blockHeader) + 7) / 8) * 8;
   
    blockHeader* bestFit = NULL;
    blockHeader* current = heap_start;
    int currentSize = 0;
    
    while (current->size_status != 1) {
        //printf("currentSize: %d\n", current->size_status);
        currentSize = (current->size_status & -8);
        int isAllocated = current->size_status & 1;

        if (isAllocated == 0 && currentSize >= blockSize) {
            // printf("currentSize inLOOP: %d\n", current->size_status);
            if (bestFit == NULL || currentSize < (bestFit->size_status & -8)) {
                bestFit = current;
                bestFit->size_status = current->size_status;
            }
        }

        current = (blockHeader*)((void*)current + currentSize);
    }

    if (bestFit == NULL || alloc_size - used < blockSize) {
	//printf("allocsize %d\n", alloc_size);
	//printf("used %d\n", used);
	//printf("blockize %d\n", blockSize);
        return NULL;
    }

    int bestFitSize = (bestFit->size_status & -8);
    int best_p_bit = (bestFit->size_status & 2);
    
    if ((bestFitSize - blockSize) > 0) {
        blockHeader *bestFitFooter = (blockHeader*)((void*)bestFit + bestFitSize - sizeof(blockHeader));
	bestFitFooter->size_status = bestFitSize;
	
        blockHeader *newBlock = (blockHeader*)((void*)bestFit + blockSize);
        bestFit->size_status = blockSize + 1 + best_p_bit;
        
	//printf("blocksize: %d\n", blockSize);
	//printf("bestFitSize: %d\n", bestFitSize);

       
       
        newBlock->size_status = bestFitSize - blockSize + 2;
	blockHeader *newBlockFooter = (blockHeader*)((void*)newBlock + (newBlock->size_status & -8) - sizeof(blockHeader));
        newBlockFooter->size_status = newBlock->size_status;
       
        
	//printf("newBlock: %d\n", newBlock->size_status);
	used += blockSize;
        //return ((void*)newBlock + sizeof(blockHeader));
    } else {
        bestFit->size_status += 1;
	used += bestFitSize;
	blockHeader *bestFitFooter = (blockHeader*)((void*)bestFit + bestFitSize - sizeof(blockHeader));
	bestFitFooter->size_status = bestFitSize;
        blockHeader *newBlock = (blockHeader*)((void*)bestFit + blockSize);  
     
        if (newBlock->size_status != 1) {
            newBlock->size_status += 2;
	    //blockHeader *newBlockFooter = (blockHeader*)((void*)newBlock + (newBlock->size_status & -8) - sizeof(blockHeader));
            //newBlockFooter->size_status = newBlock->size_status;
        }
    }
   // printf("%d\n", heap_start->size_status);
    return ((void*)bestFit + sizeof(blockHeader));
}



 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int bfree(void *ptr) {    
    if(ptr == NULL) {
    return -1;
  }
  blockHeader *freeBlock = (blockHeader*)((void*)ptr - sizeof(blockHeader));
 
  if((uintptr_t)(ptr) % 8 != 0) {
   //printf("%d\n", (uintptr_t)(ptr));
   return -1;
  }
  if(freeBlock < heap_start || freeBlock >= (blockHeader*)((void*)heap_start + alloc_size)){
	return -1;
  }
  
   int free_size = freeBlock->size_status & -8;
   //printf("free_size %d\n", free_size);
   if((freeBlock->size_status & 1) == 0) {
   return -1;
  }
  else {
     freeBlock->size_status -= 1;
  } 
 
  
  blockHeader *adj_block = (blockHeader*)((void*)freeBlock + (free_size));
  if(adj_block->size_status != 1) {
        //printf("size1: %d\n", adj_block->size_status);
	adj_block->size_status -= 2;
  }
  //printf("size2: %d\n", adj_block->size_status);
  //printf("%08x", (unsigned int)(adj_block));

  return 0;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for user-called coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    blockHeader* curr_coalesce = heap_start;
    int count = 0;
    
    
    while (curr_coalesce->size_status != 1) {
	int curr_size = curr_coalesce->size_status & -8;
        int curr_allocation = curr_coalesce->size_status & 1;
        int curr_p = curr_coalesce->size_status & 2;
	
	blockHeader *nxt_coalesce = (blockHeader*)((void*)curr_coalesce + curr_size); 
	int nxt_size = nxt_coalesce->size_status & -8;
        int nxt_allocation = nxt_coalesce->size_status & 1;

	if(curr_allocation == 0 && nxt_allocation == 0 && nxt_coalesce->size_status != 1) {
        	int new_size = nxt_size+curr_size;
		
		blockHeader *curr_footer = (blockHeader*)((void*)curr_coalesce + curr_size - sizeof(blockHeader));
        	blockHeader *nxt_footer = (blockHeader*)((void*)nxt_coalesce + nxt_size - sizeof(blockHeader));
 		//printf("%d\n", new_size);
		curr_footer->size_status = 0;
		nxt_coalesce->size_status = 0;
	
		curr_coalesce->size_status = new_size+curr_p;
		nxt_footer-> size_status = new_size;
	
		count++;
		//printf("count: %d\n", count);
	}
	nxt_coalesce = (blockHeader*)((void*)curr_coalesce + (curr_coalesce->size_status & -8)); 
	
        nxt_allocation = nxt_coalesce->size_status & 1;
	if(curr_allocation == 0 && nxt_allocation == 0) {
		continue;
	}else{
	curr_coalesce = (blockHeader*)((void*)curr_coalesce + (curr_coalesce->size_status & -8)); 
	}
    }
    return count;
}

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
} 
                  
/* 
 * Function can be used for DEBUGGING to help you visualize your heap structure.
 * Traverses heap blocks and prints info about each block found.
 * 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     
 
    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end p4Heap.c (Spring 2023)                                         



