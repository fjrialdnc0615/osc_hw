#include "allocator.h"
#include "utils.h"
#include "uart.h"

#define MEM_START 0x20000000
unsigned long *malloc_cur_ptr = (unsigned long *)MEM_START;

void* simple_malloc(size_t size){
	uint32_t size_aligned = align_len(size,4);
	unsigned long* malloc_rt_ptr = malloc_cur_ptr;
	malloc_cur_ptr += size_aligned;
	return malloc_rt_ptr;
}
