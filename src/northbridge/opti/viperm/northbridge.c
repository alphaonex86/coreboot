/* SPDX-License-Identifier: GPL-2.0-only */

#include <cbmem.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>

static struct device_operations host_bridge_ops = {
	.read_resources		= pci_dev_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.ops_pci		= &pci_dev_ops_pci,
};

static const struct pci_driver host_bridge_driver __pci_driver = {
	.ops	= &host_bridge_ops,
	.vendor	= PCI_VID_OPTI,
	.device	= PCI_DID_OPTI_82C557,
};

/* DRAM below 640 KiB and from 1 MiB, legacy I/O, VGA and BIOS areas, and the flash below 4 GiB. */
static void viperm_domain_read_resources(struct device *dev)
{
	int idx = 0;

	pci_domain_read_resources(dev);

	fixed_io_from_to_flags(dev, idx++, 0, 0x1000, IORESOURCE_RESERVE);
	ram_from_to(dev, idx++, 0, 0xa0000);
	fixed_mem_from_to_flags(dev, idx++, 0xa0000, 0xc0000, IORESOURCE_STORED);
	fixed_mem_from_to_flags(dev, idx++, 0xc0000, 0x100000, IORESOURCE_RESERVE);
	ram_from_to(dev, idx++, 0x100000, (uintptr_t)cbmem_top());
	fixed_mem_from_to_flags(dev, idx++, 0xfffe0000, 0x100000000ULL,
				IORESOURCE_RESERVE | IORESOURCE_STORED);
}

struct device_operations viperm_pci_domain_ops = {
	.read_resources	= viperm_domain_read_resources,
	.set_resources	= pci_domain_set_resources,
	.scan_bus	= pci_domain_scan_bus,
};

void mp_init_cpus(DEVTREE_CONST struct bus *cpu_bus)
{
}

struct device_operations viperm_cpu_bus_ops = {
	.read_resources	= noop_read_resources,
	.set_resources	= noop_set_resources,
	.init		= mp_cpu_bus_init,
};

struct chip_operations northbridge_opti_viperm_ops = {
	CHIP_NAME("OPTi Viper-M 82C556M/82C557M")
};
