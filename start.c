#include "efi.h"
#include "pboot.h"

void start(Handle bootloader_handle, SystemTable* system){
	system->out->output_string(system->out, u"Hello World from gcc\n\r");
	main(bootloader_handle,system);
	while(1){};
}