#include "efi.h"

LoadedImageProtocol* get_bootloader_image();
SystemTable* get_system_table();

Handle get_bootloader_handle();

void get_loaded_image();

void main(Handle, SystemTable*);//you can have warning about named main()
