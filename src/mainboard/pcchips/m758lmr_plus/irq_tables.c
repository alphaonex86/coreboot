/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * PCChips M758LMR+ IRQ routing table
 * Ported from legacy coreboot mainboard/pcchips/m758lmr+/irq_tables.c
 */

#include <arch/pirq_routing.h>
#include <device/pci_def.h>

static const struct irq_routing_table intel_irq_routing_table = {
	PIRQ_SIGNATURE,		/* u32 signature */
	PIRQ_VERSION,		/* u16 version   */
	32 + 16 * 5,		/* Total size: 5 device entries */
	0x00,			/* Bus 0 */
	PCI_DEVFN(0x01, 0),	/* ISA bridge device */
	0x0A20,			/* Reserve IRQ 11, 9, 5 for PCI */
	0x1039,			/* SIS vendor ID */
	0x0008,			/* SIS 85C503 device ID */
	0,			/* Miniport data */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Reserved */
	0xe6,			/* Checksum */
	/* clang-format off */
	{
		/* bus, devfn, {link, bitmap}x4, slot, rfu */
		{0x00, PCI_DEVFN(0x0b, 0), {{0x43, 0xdef8}, {0x44, 0xdef8}, {0x41, 0xdef8}, {0x42, 0xdef8}}, 0x01, 0x00},
		{0x00, PCI_DEVFN(0x0c, 0), {{0x44, 0xdef8}, {0x41, 0xdef8}, {0x42, 0xdef8}, {0x43, 0xdef8}}, 0x02, 0x00},
		{0x00, PCI_DEVFN(0x00, 1), {{0x61, 0xdef8}, {0x00, 0xdef8}, {0x00, 0xdef8}, {0x00, 0xdef8}}, 0x00, 0x00},
		{0x00, PCI_DEVFN(0x02, 0), {{0x41, 0xdef8}, {0x42, 0xdef8}, {0x43, 0xdef8}, {0x44, 0xdef8}}, 0x00, 0x00},
		{0x00, PCI_DEVFN(0x01, 2), {{0x41, 0xdef8}, {0x42, 0xdef8}, {0x43, 0xdef8}, {0x44, 0xdef8}}, 0x00, 0x00},
	}
	/* clang-format on */
};

unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr, &intel_irq_routing_table);
}
