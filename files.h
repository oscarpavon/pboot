#ifndef __FILES_H__
#define __FILES_H__

#include "efi.h"

void* read_file(FileProtocol* file);

uint64_t get_file_size(FileProtocol* file);

void open_file(FileProtocol** file, uint16_t* name);

void setup_file_system();

#endif
