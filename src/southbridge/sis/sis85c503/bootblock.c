/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 85C503 bootblock initialization
 * ROM decode enable, flash unlock, RTC enable
 */

#include <arch/io.h>
#include <arch/bootblock.h>
#include <device/pci_ops.h>
#include <bootblock_common.h>
#include "sis85c503.h"

void bootblock_early_southbridge_init(void)
{
	pci_devfn_t lpc_dev = PCI_DEV(0, 2, 0);

	/* Enable ROM decode (FFF80000-FFFFFFFF) */
	pci_write_config8(lpc_dev, SIS503_NVRAM_CTRL, NVRAM_DECODE_ENABLE);

	/* Enable flash write */
	pci_write_config8(lpc_dev, SIS503_MISC_CTRL, FLASH_WRITE_ENABLE);

	/* Enable RTC */
	{
		u8 val = pci_read_config8(lpc_dev, SIS503_RTC_CTRL);
		pci_write_config8(lpc_dev, SIS503_RTC_CTRL, val | 0x10);
	}
}
