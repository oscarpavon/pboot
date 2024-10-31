
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned long long int uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t ;
typedef unsigned char uint8_t ;

#define bool uint8_t
#define true 1
#define false 0

#define __UINT64_C(c)	c ## UL
#define UINT64_MAX		(__UINT64_C(18446744073709551615))

typedef struct {
	uint16_t *entry_name;
	uint16_t *kernel_name;
	uint16_t *kernel_parameters;
}BootLoaderEntry;

#endif
