#include "types.h"

static uint8_t default_entry = 2;

static BootLoaderEntry entries[6] = {
	{u"", u"vmlinuz", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/pinit"},
	{u"", u"vmlinuz", u"root=/dev/nvme0n1p3 rw fstype=ext4 init=/pinit"},
	{u"", u"vmlinuz", u"root=/dev/nvme0n1p3 rw fstype=ext4"},
	{u"", u"ubuntu-linux", u"root=/dev/nvme0n1p2 ro quiet fstype=ext4 splash vt.handoff=7 initrd=ubuntu-initrd"},
	{u"", u"pkernel", u""},
	{u"", u"vmlinuz", u"root=/dev/sda1 rw fstype=vfat init=/sbin/pinit"},
};
