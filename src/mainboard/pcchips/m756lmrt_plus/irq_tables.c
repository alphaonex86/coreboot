/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * PIRQ routing table extracted from the factory AMI BIOS of the
 * PCChips M756LMRT+ V5.3 (image 010925s.rom, AMIBIOSC0627, 11/01/01).
 * Router is the SiS 85C503 LPC/ISA bridge at bus 0, dev 1, fn 0.
 */

#include <arch/pirq_routing.h>
#include <device/pci_def.h>

static const struct irq_routing_table intel_irq_routing_table = {
	PIRQ_SIGNATURE,		/* u32 signature */
	PIRQ_VERSION,		/* u16 version   */
	32 + 16 * 5,		/* Total size: 5 device entries */
	0x00,			/* Bus 0 */
	PCI_DEVFN(0x01, 0),	/* Interrupt router: SiS 85C503 (00:01.0) */
	0x0000,			/* No PCI-exclusive IRQs */
	0x1039,			/* SiS vendor ID */
	0x0008,			/* SiS 85C503 device ID */
	0,			/* Miniport data */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Reserved */
	0xcc,			/* Checksum (makes table sum to 0 mod 256) */
	/* clang-format off */
	{
		/* bus, devfn, {link, bitmap}x4, slot, rfu */
		{0x00, PCI_DEVFN(0x01, 0), {{0x41, 0xdeb8}, {0x42, 0xdeb8}, {0x43, 0xdeb8}, {0x44, 0xdeb8}}, 0x00, 0x00},
		{0x00, PCI_DEVFN(0x02, 0), {{0x41, 0xdeb8}, {0x42, 0xdeb8}, {0x43, 0xdeb8}, {0x44, 0xdeb8}}, 0x00, 0x00},
		{0x00, PCI_DEVFN(0x09, 0), {{0x41, 0xceb8}, {0x42, 0xceb8}, {0x43, 0xceb8}, {0x44, 0xceb8}}, 0x01, 0x00},
		{0x00, PCI_DEVFN(0x0b, 0), {{0x44, 0xceb8}, {0x41, 0xceb8}, {0x42, 0xceb8}, {0x43, 0xceb8}}, 0x02, 0x00},
		{0x00, PCI_DEVFN(0x0f, 0), {{0x42, 0xceb8}, {0x43, 0xceb8}, {0x41, 0xceb8}, {0x42, 0xceb8}}, 0x00, 0x00},
	}
	/* clang-format on */
};

unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr, &intel_irq_routing_table);
}
