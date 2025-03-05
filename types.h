
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	uint16_t entry_name[20];
	uint16_t kernel_name[20];
	uint16_t kernel_parameters[100];
}BootLoaderEntry;

#endif
