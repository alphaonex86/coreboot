/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <commonlib/helpers.h>
#include <device/pci_ops.h>

#include "82c558m.h"

/* The IBM BIOS's values (POST 86h), PIRQ routing apart: ISA timing, pin functions, the IDE on. */
static const u8 init_table[][2] = {
	{ 0x42, 0x01 },	/* DREQ6 pin is EPMI0# */
	{ 0x43, 0x08 },	/* no delay between back-to-back ISA cycles */
	{ 0x44, 0x03 },	/* encoded EDACK, MP[7:4] on the 82C558M, PIRQ[3:2]# */
	{ 0x46, 0x40 },	/* XDIR only for ROM/NVRAM */
	{ 0x48, 0x10 },	/* DREQ1 pin = DREQ1/6 */
	{ 0x4f, 0x60 },	/* IDE on */
	{ 0x52, 0x04 },
};

void opti_82c558m_early_init(unsigned int bus_mhz)
{
	for (int i = 0; i < ARRAY_SIZE(init_table); i++)
		pci_write_config8(ISA_BRIDGE, init_table[i][0], init_table[i][1]);
	/* ATCLK: LCLK / 3 on a 50 MHz bus (25 MHz PCI), else LCLK / 4 */
	pci_write_config8(ISA_BRIDGE, CYCLE_CTL, bus_mhz == 50 ? 0x10 : 0x00);

	/* DRAM refresh request every 15 us: PIT channel 1, mode 2, count 18; refresh page 0 */
	outb(0x54, 0x43);
	outb(0x12, 0x41);
	outb(0x00, 0x8f);
}
