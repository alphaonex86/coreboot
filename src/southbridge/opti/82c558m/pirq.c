/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/pirq_routing.h>
#include <device/pci_ops.h>

#include "82c558m.h"

/* PIRQ0-3# (route[0..3], an IRQ or 0) to 40h-41h, the IRQs level-triggered in 42h. */
void pirq_assign_irqs(const unsigned char route[CONFIG_MAX_PIRQ_LINKS])
{
	static const u8 irqs[8] = { 0, 5, 9, 10, 11, 12, 14, 15 };
	u16 links = pci_s_read_config16(ISA_BRIDGE, PIRQ_ROUTE) & 0x3000;
	u8 level = pci_s_read_config8(ISA_BRIDGE, MISC_1) & 0x01;

	for (int link = 0; link < 4; link++)
		for (int code = 1; code < ARRAY_SIZE(irqs); code++)
			if (route[link] == irqs[code]) {
				links |= code << (3 * link);
				level |= 1 << code;
			}
	pci_s_write_config16(ISA_BRIDGE, PIRQ_ROUTE, links);
	pci_s_write_config8(ISA_BRIDGE, MISC_1, level);
}
