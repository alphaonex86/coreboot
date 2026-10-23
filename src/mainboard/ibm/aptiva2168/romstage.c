/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <northbridge/opti/viperm/viperm.h>

#include "aptiva2168.h"

/* JP2 on the FDC37C93x GP21-GP20, read through its run-mode GPIO port (CR03 = 83h, as the IBM BIOS sets it:
   index EAh, data EBh, GP2x at index 2): 00 66 MHz, 10 60 MHz, 11 50 MHz. */
unsigned int mainboard_bus_mhz(void)
{
	static const u8 mhz[4] = { 66, 0, 60, 50 };

	outb(0x55, SUPERIO_PORT);
	outb(0x55, SUPERIO_PORT);
	outb(0x03, SUPERIO_PORT);
	outb(0x83, SUPERIO_PORT + 1);
	outb(0xaa, SUPERIO_PORT);

	outb(0x02, 0xea);
	return mhz[inb(0xeb) & 3];
}
