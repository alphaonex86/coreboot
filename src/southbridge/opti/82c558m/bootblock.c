/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/bootblock.h>
#include <device/pci_ops.h>

#define ISA_BRIDGE	PCI_DEV(0, 1, 0)
#define ROMCS_HIGH	0x4b	/* ROMCS# below 4 GiB, one bit per 32 KiB */

/* The whole 128 KiB flash below 4 GiB, for CBFS: at reset ROMCS# covers its top 64 KiB only. */
void bootblock_early_southbridge_init(void)
{
	pci_or_config8(ISA_BRIDGE, ROMCS_HIGH, 0x30);
}
