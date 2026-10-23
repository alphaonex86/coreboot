/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/pirq_routing.h>
#include <device/pci_ids.h>

/*
 * The riser's slots 07h, 0Bh and 0Ch; links 1-4 are the 82C558M's PIRQ0-3#. INTA: 0Bh on PIRQ0#, 07h and 0Ch on
 * PIRQ1#, routed to IRQ10 and IRQ11 as by the IBM BIOS. INTB-INTD follow the IBM BIOS's swizzle (A<->B, C<->D
 * on 07h and 0Ch), not seen on hardware; PIRQ2#/3# share IRQ10/11 (inferred).
 */
#define IRQ10	(1 << 10)
#define IRQ11	(1 << 11)

static const struct irq_routing_table aptiva2168_irq_routing_table = {
	PIRQ_SIGNATURE,
	PIRQ_VERSION,
	32 + 16 * CONFIG_IRQ_SLOT_COUNT,
	0x00,			/* router bus */
	(0x01 << 3) | 0x0,	/* router device: 82C558M */
	0,			/* IRQs only for PCI */
	PCI_VID_OPTI,
	PCI_DID_OPTI_82C558,
	0,			/* miniport data */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	0xca,			/* checksum */
	{
		/* bus, dev|fn, {link, bitmap} INTA-INTD, slot, rfu */
		{0x00, (0x07 << 3) | 0x0, {{2, IRQ11}, {1, IRQ10}, {4, IRQ11}, {3, IRQ10}}, 0x1, 0x0},
		{0x00, (0x0b << 3) | 0x0, {{1, IRQ10}, {2, IRQ11}, {3, IRQ10}, {4, IRQ11}}, 0x2, 0x0},
		{0x00, (0x0c << 3) | 0x0, {{2, IRQ11}, {1, IRQ10}, {4, IRQ11}, {3, IRQ10}}, 0x3, 0x0},
	}
};

unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr, &aptiva2168_irq_routing_table);
}
