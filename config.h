#include "types.h"

uint8_t default_entry = 0;
bool show_menu = false;

static const BootLoaderEntry entries[] = {
	{u"Pavon Linux", u"pLinux", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/pinit"},
	{u"Gentoo", u"pLinux", u"quiet root=/dev/nvme0n1p2 ro fstype=ext4"},
	{u"PKernel", u"pboot", u""},
	{u"Asmboot", u"asmboot", u""},
	{u"Pavon VM", u"pavonlinuz", u"quiet root=/dev/sda2 rw fstype=ext4 init=/sbin/pinit"},
	{u"Pavon Linux USB", u"vmlinuz",
		u"rootwait root=PARTUUID=45e8184f-fd5c-4e77-a94e-b16a4a9823cb rw fstype=ext4"},
	{u"Pavon stinit quiet initcall debug", u"pavonlinuz", u"initcall_debug quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/sbin/pinit"},
	{u"Pavon sinit usb off", u"vmlinuz", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/sbin/pinit usbcore.authorized_default=0"},
	{u"Pavon /sbin/pinit", u"vmlinuz", u"root=/dev/nvme0n1p3 rw fstype=ext4 init=/sbin/pinit"},
	{u"Pavon sinit", u"pavonlinuz", u"quiet root=/dev/nvme0n1p3 rw fstype=ext4 init=/sbin/sinit"},
};
