#include "efi.h"
void pboot(Handle bootloader_handle, SystemTable* system){
	system->out->output_string(system->out, u"Hello World from gcc");
	while(1){};
}
