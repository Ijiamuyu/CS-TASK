#ifndef __MMALLOC_H__
#define __MMALLOC_H__

#include <stdio.h>
#include <stdint.h>

void memory_init(void);

void *mmalloc(uint32_t size);

void mfree(void *p);

void print_malloc_info(void);

#endif /* __MMALLOC_H__ */
