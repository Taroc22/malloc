#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT 		8u
#define ALIGN_UP(x,a)	(((x)+((a)-1)) & ~((a)-1))
#define MIN_PAYLOAD 	ALIGNMENT

//sbrk deprecated; only unix like systems

typedef struct block {
	size_t size;
	int free; // 1=free; 0=used;
	struct block *next;
} block_t;

#define BLOCK_SIZE		ALIGN_UP(sizeof(block_t), ALIGNMENT)
#define MIN_SPLIT		(BLOCK_SIZE + MIN_PAYLOAD)

static block_t *heap_start = NULL;


static block_t *find_free_block(block_t **last, size_t size){
	block_t *cur = heap_start;
	if (last) *last = NULL;
	
	while (cur){
		if (cur->free && cur->size >= size)
			return cur;
		
		if (last) *last = cur;
		cur = cur->next;	
	}
	return NULL;
}


static block_t *extend_heap(block_t *last, size_t size){
	if (size > SIZE_MAX - BLOCK_SIZE)
		return NULL;

	size_t total = size + BLOCK_SIZE;

	if (total > (size_t)INTPTR_MAX)
		return NULL;

	void *prev_break = sbrk(0);
	if (prev_break == (void*)-1)
		return NULL;

	void *request = sbrk((intptr_t)total);
	if (request == (void*)-1)
		return NULL;

	block_t *block = (block_t *)prev_break;
	block->size = size;
	block->free = 0;
	block->next = NULL;

	if (last)
		last->next = block;

	return block;
}


static void split_block(block_t *block, size_t size) {
	block_t *new_block = (block_t *)((char *)block + BLOCK_SIZE + size);
	new_block->size = block->size - size - BLOCK_SIZE;
	new_block->free = 1;
	new_block->next = block->next;
	
	block->size = size;
	block->next = new_block;
}


static inline block_t *get_block_ptr(void *ptr){
    return (block_t *)((char *)ptr - BLOCK_SIZE);
}


void *malloc2(size_t size){
	block_t *block;
	block_t *last = NULL;
	
	if (size == 0)
		return NULL;
	
	size = ALIGN_UP(size, ALIGNMENT);
	if (size > SIZE_MAX - BLOCK_SIZE) return NULL;
	
	if (!heap_start) {
		block = extend_heap(NULL, size);
		if (!block)
			return NULL;
		heap_start = block;
	} else {
		block = find_free_block(&last, size);
		if (!block) {
			block = extend_heap(last, size);
			if (!block)
				return NULL;
		} else {
			if (block->size >= size + MIN_SPLIT)
				split_block(block, size);
			block->free = 0;
		}
	}
	return (void *)((char *)block + BLOCK_SIZE);
}


void free2(void *ptr){
	if (!ptr)
		return;
	
	block_t *block = get_block_ptr(ptr);
	
	if (block->free)
		return;
	block->free = 1;
	
	/*Coalescing with next block*/
	if (block->next && block->next->free){
		block->size += BLOCK_SIZE + block->next->size;
		block->next = block->next->next;
	}
}