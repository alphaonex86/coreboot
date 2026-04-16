/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 85C503/5513 ISA/LPC bridge driver
 * Ported from legacy coreboot southbridge.c
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include <pc80/i8254.h>
#include "sis85c503.h"

static void keyboard_on(struct device *dev)
{
	u8 val;

	/* Enable PS/2 keyboard and mouse */
	val = pci_read_config8(dev, SIS503_KBC_CTRL);
	val |= KBC_MOUSE_ENABLE;
	pci_write_config8(dev, SIS503_KBC_CTRL, val);

	printk(BIOS_DEBUG, "SIS 503: keyboard/mouse enabled\n");
}

static void nvram_on(struct device *dev)
{
	/* Enable flash/NVRAM decode range FFF80000-FFFFFFFF */
	pci_write_config8(dev, SIS503_NVRAM_CTRL, NVRAM_DECODE_ENABLE);
	/* Enable flash write */
	pci_write_config8(dev, SIS503_MISC_CTRL, FLASH_WRITE_ENABLE);

	printk(BIOS_DEBUG, "SIS 503: NVRAM/flash enabled\n");
}

static void serial_irq_fixup(struct device *dev)
{
	/* Enable serial IRQ */
	pci_write_config8(dev, SIS503_SIRQ_CTRL, SIRQ_ENABLE);
	/* Enable all IRQs */
	pci_write_config8(dev, SIS503_IRQ_ENABLE1, IRQ_ALL_LOW);
	pci_write_config8(dev, SIS503_IRQ_ENABLE2, IRQ_ALL_HIGH);

	printk(BIOS_DEBUG, "SIS 503: serial IRQ enabled\n");
}

static void rtc_fixup(struct device *dev)
{
	u8 val;

	/* Enable internal RTC */
	val = pci_read_config8(dev, SIS503_RTC_CTRL);
	val |= 0x10;
	pci_write_config8(dev, SIS503_RTC_CTRL, val);

	/* Set RTC clock divider (978us interrupt rate) */
	outb(RTC_REG_A, RTC_INDEX);
	outb(0x26, RTC_DATA);

	/* Set 24-hour time format */
	outb(RTC_REG_B, RTC_INDEX);
	outb(0x02, RTC_DATA);

	/* Clear checksum error */
	outb(RTC_REG_D, RTC_INDEX);
	inb(RTC_DATA);

	printk(BIOS_DEBUG, "SIS 503: RTC enabled\n");
}

static void sis85c503_init(struct device *dev)
{
	printk(BIOS_DEBUG, "SIS 85C503 ISA bridge init\n");

	serial_irq_fixup(dev);
	nvram_on(dev);
	keyboard_on(dev);
	rtc_fixup(dev);

	/* Set up 8254 timer */
	setup_i8254();
}

static void sis85c503_enable(struct device *dev)
{
	printk(BIOS_DEBUG, "SIS 85C503: %s %s\n",
	       dev_path(dev), dev->enabled ? "enabled" : "disabled");
}

static struct device_operations isa_bridge_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = sis85c503_init,
	.enable           = sis85c503_enable,
	.scan_bus         = scan_static_bus,
};

static const struct pci_driver isa_bridge_driver __pci_driver = {
	.ops    = &isa_bridge_ops,
	.vendor = PCI_VID_SIS,
	.device = SIS85C503_DEVID,
};

struct chip_operations southbridge_sis_sis85c503_ops = {
	.name = "SIS 85C503 ISA Bridge",
};
