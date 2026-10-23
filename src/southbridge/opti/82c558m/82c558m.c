/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>

/* The chipset's index/data ports 22h-24h; with a resource, the allocator also sets the children's: the Super I/O's
   logical devices. */
static void isa_bridge_read_resources(struct device *dev)
{
	pci_dev_read_resources(dev);
	fixed_io_from_to_flags(dev, 0, 0x22, 0x25, IORESOURCE_RESERVE);
}

static struct device_operations isa_bridge_ops = {
	.read_resources		= isa_bridge_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.scan_bus		= scan_static_bus,
};

static const struct pci_driver isa_bridge_driver __pci_driver = {
	.ops	= &isa_bridge_ops,
	.vendor	= PCI_VID_OPTI,
	.device	= PCI_DID_OPTI_82C558,
};

/* The MIDE IDE controller (00:14.0) in legacy mode, both channels, IRQ14/15, as the IBM BIOS leaves it. */
static void mide_init(struct device *dev)
{
	pci_write_config8(dev, PCI_CLASS_PROG, 0x8a);
	pci_write_config8(dev, PCI_INTERRUPT_LINE, 14);
}

static struct device_operations mide_ops = {
	.read_resources		= pci_dev_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init			= mide_init,
};

static const struct pci_driver mide_driver __pci_driver = {
	.ops	= &mide_ops,
	.vendor	= PCI_VID_OPTI,
	.device	= PCI_DID_OPTI_82C621,
};
