/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/bootblock.h>
#include <device/pci_ops.h>

#include "82c558m.h"

/* The whole 128 KiB flash at E0000 (romstage runs there) and below 4 GiB (CBFS): at reset ROMCS# covers its
   top 64 KiB only; a CPU-only reset keeps the last boot's C0h (no flash at E0000-FFFFF). */
void bootblock_early_southbridge_init(void)
{
	pci_write_config8(ISA_BRIDGE, ROMCS_LOW, 0x30);
	pci_or_config8(ISA_BRIDGE, ROMCS_HIGH, 0x30);
}
