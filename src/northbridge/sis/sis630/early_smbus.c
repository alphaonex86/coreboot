/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 630 early SMBus routines for SPD reads during romstage
 * SMBus controller is part of the SIS 85C503 ISA bridge.
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <delay.h>
#include "sis630.h"

#define ACPI_BASE		0x7550

/* SMBus registers (relative to ACPI base) */
#define SMBHSTSTS		(ACPI_BASE + 0x80)
#define SMBHSTCNT		(ACPI_BASE + 0x81)
#define SMBHSTCMD		(ACPI_BASE + 0x82)
#define SMBXMITADD		(ACPI_BASE + 0x83)
#define SMBHSTDAT0		(ACPI_BASE + 0x84)

#define SMBHST_BUSY		(1 << 0)
#define SMBHST_DONE		(1 << 1)
#define SMBHST_ERROR		(1 << 2)

void sis630_early_smbus_init(void)
{
	pci_devfn_t lpc_dev = PCI_DEV(0, 2, 0);

	/* Enable ACPI/SMBus by setting ACPI base address */
	pci_write_config16(lpc_dev, 0x74, ACPI_BASE);
	/* Enable ACPI space */
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

int sis630_smbus_read_byte(u8 device, u8 address)
{
	/* Clear status */
	outb(0xff, SMBHSTSTS);

	/* Set slave address (read) */
	outb((device << 1) | 1, SMBXMITADD);

	/* Set command/offset */
	outb(address, SMBHSTCMD);

	/* Start byte data read */
	outb(0x48, SMBHSTCNT);

	if (smbus_wait() != 0) {
		printk(BIOS_DEBUG, "SMBus read failed: dev=%02x addr=%02x\n",
		       device, address);
		return -1;
	}

	return inb(SMBHSTDAT0);
}
