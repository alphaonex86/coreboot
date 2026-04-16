/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <cpu/x86/mp.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include "sis735.h"

static const int ramsizes[16] = {
	8, 32, 32, 64, 16, 64, 64, 128,
	32, 128, 128, 256, 16, 256, 256, 512
};

static void northbridge_init(struct device *dev)
{
	printk(BIOS_DEBUG, "SIS 735 northbridge init\n");
}

static void sis735_domain_set_resources(struct device *dev)
{
	pci_devfn_t nb_dev = PCI_DEV(0, 0, 0);
	int total_kb = 0;
	int slot;
	u8 bank_en;

	bank_en = pci_s_read_config8(nb_dev, SIS735_REG_BANK_EN);

	for (slot = 0; slot < 4; slot++) {
		u8 bank_val;
		int size_idx, banks;

		if (!(bank_en & (1 << slot)))
			continue;

		bank_val = pci_s_read_config8(nb_dev, SIS735_REG_BANK0 + slot);
		size_idx = bank_val & 0x1f;
		if (size_idx >= 16)
			size_idx = 0;
		banks = (bank_val & 0x20) ? 2 : 1;

		total_kb += ramsizes[size_idx] * banks * 1024;
	}

	printk(BIOS_DEBUG, "SIS 735: %d KB RAM\n", total_kb);

	ram_resource_kb(dev, 0, 0, 640);
	ram_resource_kb(dev, 1, 1024, total_kb - 1024);

	assign_resources(dev->downstream);
}

static struct device_operations pci_domain_ops = {
	.read_resources   = pci_domain_read_resources,
	.set_resources    = sis735_domain_set_resources,
	.scan_bus         = pci_host_bridge_scan_bus,
};

static int get_cpu_count(void)
{
	return CONFIG_MAX_CPUS;
}

static const struct mp_ops mp_ops = {
	.get_cpu_count = get_cpu_count,
};

void mp_init_cpus(struct bus *cpu_bus)
{
	mp_init_with_smm(cpu_bus, &mp_ops);
}

static struct device_operations cpu_bus_ops = {
	.read_resources   = noop_read_resources,
	.set_resources    = noop_set_resources,
	.init             = mp_cpu_bus_init,
};

static void enable_dev(struct device *dev)
{
	if (dev->path.type == DEVICE_PATH_DOMAIN)
		dev->ops = &pci_domain_ops;
	else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER)
		dev->ops = &cpu_bus_ops;
}

static void northbridge_pci_init(struct device *dev)
{
	northbridge_init(dev);
}

static struct device_operations northbridge_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = northbridge_pci_init,
};

static const struct pci_driver northbridge_driver __pci_driver = {
	.ops    = &northbridge_ops,
	.vendor = PCI_VID_SIS,
	.device = SIS735_DEVID,
};

struct chip_operations northbridge_sis_sis735_ops = {
	.name = "SIS 735 Northbridge",
	.enable_dev = enable_dev,
};
