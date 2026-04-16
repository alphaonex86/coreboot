/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 730 early SMBus routines for SPD reads during romstage
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <delay.h>
#include "sis730.h"

#define ACPI_BASE		0x7550

#define SMBHSTSTS		(ACPI_BASE + 0x80)
#define SMBHSTCNT		(ACPI_BASE + 0x81)
#define SMBHSTCMD		(ACPI_BASE + 0x82)
#define SMBXMITADD		(ACPI_BASE + 0x83)
#define SMBHSTDAT0		(ACPI_BASE + 0x84)

#define SMBHST_BUSY		(1 << 0)
#define SMBHST_DONE		(1 << 1)
#define SMBHST_ERROR		(1 << 2)

void sis730_early_smbus_init(void)
{
	pci_devfn_t lpc_dev = PCI_DEV(0, 2, 0);

	pci_write_config16(lpc_dev, 0x74, ACPI_BASE);
	pci_write_config8(lpc_dev, 0x40, pci_read_config8(lpc_dev, 0x40) | 0x80);
}

static int smbus_wait(void)
{
	int timeout = 1000;

	while (timeout--) {
		u8 status = inb(SMBHSTSTS);
		if (status & SMBHST_ERROR)
			return -1;
		if (status & SMBHST_DONE)
			return 0;
		udelay(10);
	}
	return -1;
}

int sis730_smbus_read_byte(u8 device, u8 address)
{
	outb(0xff, SMBHSTSTS);
	outb((device << 1) | 1, SMBXMITADD);
	outb(address, SMBHSTCMD);
	outb(0x48, SMBHSTCNT);

	if (smbus_wait() != 0) {
		printk(BIOS_DEBUG, "SMBus read failed: dev=%02x addr=%02x\n",
		       device, address);
		return -1;
	}

	return inb(SMBHSTDAT0);
}
