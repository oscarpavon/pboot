#include "types.h"


bool show_bootloader = true;
uint8_t default_entry = 1;

static const BootLoaderEntry entries[] = {
	{u"PavonLinux"},
	{u"Gentoo"}
};
