#ifndef __UTILS__
#define __UTILS__

#include "types.h"

void hang();

size_t u16strlen(const uint16_t *str);

void *set_memory(void *pointer, int value, size_t size);

void log(uint16_t* text);

void *copy_memory(void *destination, const void *source, size_t size);

void allocate_memory(uint64_t size, void** memory);

#endif
