
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t ;
typedef unsigned char uint8_t ;

#define bool _Bool
#define true 1
#define false 0

# define UINT64_MAX         18446744073709551615

typedef struct {
	uint16_t *name;
}BootLoaderEntry;

#endif
