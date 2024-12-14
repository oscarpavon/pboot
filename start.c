#include "efi.h"
void start(Handle bootloader_handle, SystemTable* system){
	system->out->output_string(system->out, u"Hello World from gcc");
	while(1){};
}
