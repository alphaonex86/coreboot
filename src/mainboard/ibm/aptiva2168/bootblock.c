/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <device/pnp_ops.h>
#include <superio/smsc/smscsuperio/smscsuperio.h>

#define SUPERIO_PORT	0x370
#define KBC_DEV		PNP_DEV(SUPERIO_PORT, 7)

void bootblock_mainboard_early_init(void)
{
	/* The FDC37C93x keyboard controller first, as the IBM BIOS does: IRQ1, IRQ12 */
	outb(0x55, SUPERIO_PORT);
	outb(0x55, SUPERIO_PORT);
	pnp_set_logical_device(KBC_DEV);
	pnp_set_irq(KBC_DEV, PNP_IDX_IRQ0, 1);
	pnp_set_irq(KBC_DEV, PNP_IDX_IRQ1, 12);
	pnp_set_enable(KBC_DEV, 1);
	outb(0xaa, SUPERIO_PORT);

	smscsuperio_enable_serial(PNP_DEV(SUPERIO_PORT, SMSCSUPERIO_SP1), 0x3f8);
}
