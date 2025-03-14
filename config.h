#include "types.h"

static uint8_t default_entry = 2;

static bool show_menu = false;

static BootLoaderEntry entries[4] = {
	{u"Pavon Linux", u"pLinux", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/pinit"},
	{u"Ubuntu", u"ubuntu-linux", u"root=/dev/nvme0n1p2 ro quiet fstype=ext4 splash vt.handoff=7 initrd=ubuntu-initrd"},
	{u"PKernel", u"pkernel", u""},
	{u"Pavon VM", u"pavonlinuz", u"quiet root=/dev/sda2 rw fstype=ext4 init=/sbin/pinit"},
};
