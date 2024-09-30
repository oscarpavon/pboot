#include "types.h"

uint8_t default_entry = 0;

static const BootLoaderEntry entries[] = {
	{u"Pavon", u"vmlinuz", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4"},
	{u"Gentoo", u"vmlinuz", u"quiet root=/dev/nvme0n1p2 ro fstype=ext4"},
};
