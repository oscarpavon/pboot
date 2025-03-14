#include "utils.h"
#include "efi.h"
#include "pboot.h"

void allocate_memory(uint64_t size, void** memory){

	SystemTable* system_table = get_system_table();
	system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA, size, memory);
}

void log(uint16_t* text){
	SystemTable* system_table = get_system_table();
	
	system_table->out->output_string(system_table->out,text);
	system_table->out->output_string(system_table->out,u"\n\r");

}


size_t u16strlen(const uint16_t *str)
{
	const uint16_t *pos = str;

	while (*pos++)
		;
	return pos - str - 1;
}

void *set_memory(void *pointer, int value, size_t size)
{
	char *to = pointer;

	for (size_t i = 0; i < size; ++i)
		*to++ = value;
	return pointer;
}

void *copy_memory(void *destination, const void *source, size_t size)
{
	const char *from = source;
	char *to = destination;

	for (size_t i = 0; i < size; ++i)
		*to++ = *from++;
	return destination;
}


void hang(){
	while (1) {};
}
