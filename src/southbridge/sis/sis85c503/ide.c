/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 5513 IDE controller driver
 * Ported from legacy coreboot southbridge.c ide_fixup()
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include "sis85c503.h"

static void ide_init(struct device *dev)
{
	printk(BIOS_DEBUG, "SIS 5513 IDE init\n");

	/* Enable I/O and memory access + bus master */
	pci_write_config16(dev, IDE_CMD_REG, 0x0007);

	/* Clear abort status */
	pci_write_config16(dev, IDE_STATUS_REG, 0x3000);

	/* Programming interface: IDE compatible mode */
	pci_write_config8(dev, IDE_PI_REG, 0x0A);

	/* Latency timer */
	pci_write_config8(dev, IDE_LATENCY, 0x10);

	/* IRQ 14 */
	pci_write_config8(dev, IDE_IRQ, 0x0E);

	/* Primary master data recovery time (PIO Mode 2) */
	pci_write_config8(dev, IDE_PRIMARY_TIMING, 0x04);

	/* Primary master data active time (UDMA Mode 2) */
	pci_write_config8(dev, IDE_PRIMARY_ACTIVE, 0xB4);

	/* Read/Write FIFO Threshold = 1/4 */
	pci_write_config8(dev, IDE_FIFO_THRESHOLD, 0x05);

	/* Enable Bus Master, IDE 0/1 */
	{
		u8 val = pci_read_config8(dev, IDE_BM_CTRL);
		pci_write_config8(dev, IDE_BM_CTRL, val | 0xE6);
	}

	/* Postwrite/Prefetch for IDE0 master */
	pci_write_config8(dev, IDE_PREFETCH, 0x11);

	/* Prefetch count primary */
	pci_write_config16(dev, IDE_PRI_PREFETCH_CNT, 0x0200);

	/* Prefetch count secondary */
	pci_write_config16(dev, IDE_SEC_PREFETCH_CNT, 0x0200);

	/* Miscellaneous control */
	{
		u8 val = pci_read_config8(dev, IDE_MISC_CTRL);
		pci_write_config8(dev, IDE_MISC_CTRL, (val & 0xFF) | 0x14);
	}

	printk(BIOS_DEBUG, "SIS 5513 IDE init done\n");
}

static struct device_operations ide_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = ide_init,
};

static const struct pci_driver ide_driver __pci_driver = {
	.ops    = &ide_ops,
	.vendor = PCI_VID_SIS,
	.device = SIS5513_DEVID,
};
