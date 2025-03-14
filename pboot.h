#include "efi.h"

#include "types.h"
#include "files.h"
#include "utils.h"
#include "configuration.h"

LoadedImageProtocol* get_bootloader_image();
SystemTable* get_system_table();

Handle get_bootloader_handle();

void get_loaded_image();

void boot();

void set_default_entry(uint8_t entry);

void main(Handle, SystemTable*);//you can have warning about named main()
