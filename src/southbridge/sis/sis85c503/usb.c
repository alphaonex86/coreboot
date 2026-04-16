/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 7001 USB OHCI controller driver
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include "sis85c503.h"

static void usb_init(struct device *dev)
{
	printk(BIOS_DEBUG, "SIS 7001 USB OHCI init\n");

	/* Enable bus master + memory space */
	{
		u16 cmd = pci_read_config16(dev, 0x04);
		pci_write_config16(dev, 0x04, cmd | 0x0006);
	}
}

static struct device_operations usb_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = usb_init,
};

static const struct pci_driver usb_driver __pci_driver = {
	.ops    = &usb_ops,
	.vendor = PCI_VID_SIS,
	.device = SIS7001_DEVID,
};
