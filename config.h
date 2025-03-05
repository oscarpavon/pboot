#include "types.h"

static uint8_t default_entry = 0;
static bool show_menu = true;

static BootLoaderEntry entries[4] = {
	{u"Pavon Linux", u"pLinux", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/pinit"},
	{u"Gentoo", u"pLinux", u"quiet root=/dev/nvme0n1p2 ro fstype=ext4"},
	{u"PKernel", u"pkernel", u""},
};
