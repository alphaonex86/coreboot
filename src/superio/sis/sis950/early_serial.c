/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 950 early serial port initialization
 * Ported from legacy coreboot setup_serial.inc
 */

#include <arch/io.h>
#include <device/pnp_def.h>
#include "sis950.h"

void sis950_enable_serial(pnp_devfn_t dev, u16 iobase)
{
	u16 port = dev >> 8;

	/* Enter PnP config mode */
	outb(SIS950_UNLOCK_1, port);
	outb(SIS950_UNLOCK_2, port);
	outb(SIS950_UNLOCK_3, port);
	outb(SIS950_UNLOCK_4, port);

	/* Select COM1 (LDN 4) */
	outb(SIS950_PNP_LDN, port);
	outb(SIS950_COM1, port + 1);

	/* Set clock to 24MHz */
	outb(SIS950_PNP_CLOCK, port);
	outb(inb(port + 1) | 0x04, port + 1);

	/* Set I/O base address */
	outb(SIS950_PNP_IOBASE_HI, port);
	outb((iobase >> 8) & 0xff, port + 1);
	outb(SIS950_PNP_IOBASE_LO, port);
	outb(iobase & 0xff, port + 1);

	/* Set IRQ 4 */
	outb(SIS950_PNP_IRQ, port);
	outb(0x04, port + 1);

	/* Activate device */
	outb(SIS950_PNP_ACTIVATE, port);
	outb(0x01, port + 1);

	/* Exit PnP config mode */
	outb(SIS950_PNP_CONFIG, port);
	outb(0x02, port + 1);
}
