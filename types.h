
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint16_t Unicode;

typedef struct {
	Unicode entry_name[20];
	Unicode kernel_name[20];
	Unicode kernel_parameters[100];
}BootLoaderEntry;

#endif
