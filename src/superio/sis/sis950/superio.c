/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 950 Super I/O driver
 * Ported from legacy coreboot superio/sis/950/superio.c
 */

#include <device/device.h>
#include <device/pnp.h>
#include <pc80/keyboard.h>
#include "sis950.h"

static void sis950_init(struct device *dev)
{
	if (!dev->enabled)
		return;

	switch (dev->path.pnp.device) {
	case SIS950_KBC:
		pc_keyboard_init(NO_AUX_DEVICE);
		break;
	}
}

/*
 * SIS 950 PnP config mode:
 * Enter: write 0x87, 0x01, 0x55, 0x55 to config port
 * Exit:  write 0x02 to config port, 0x02 to data port
 */
static void sis950_pnp_enter_conf_mode(struct device *dev)
{
	u16 port = dev->path.pnp.port;

	outb(0x87, port);
	outb(0x01, port);
	outb(0x55, port);
	outb(0x55, port);
}

static void sis950_pnp_exit_conf_mode(struct device *dev)
{
	u16 port = dev->path.pnp.port;

	outb(0x02, port);
	outb(0x02, port + 1);
}

static const struct pnp_mode_ops sis950_pnp_mode_ops = {
	.enter_conf_mode = sis950_pnp_enter_conf_mode,
	.exit_conf_mode  = sis950_pnp_exit_conf_mode,
};

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = pnp_set_resources,
	.enable_resources = pnp_enable_resources,
	.enable           = pnp_alt_enable,
	.init             = sis950_init,
	.ops_pnp_mode     = &sis950_pnp_mode_ops,
};

static struct pnp_info pnp_dev_info[] = {
	{ NULL, SIS950_FDC,  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, 0x07f8, },
	{ NULL, SIS950_COM1, PNP_IO0 | PNP_IRQ0, 0x07f8, },
	{ NULL, SIS950_COM2, PNP_IO0 | PNP_IRQ0, 0x07f8, },
	{ NULL, SIS950_KBC,  PNP_IO0 | PNP_IO1 | PNP_IRQ0, 0x07ff, 0x07ff, },
};

static void enable_dev(struct device *dev)
{
	pnp_enable_devices(dev, &ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}

struct chip_operations superio_sis_sis950_ops = {
	.name = "SIS 950 Super I/O",
	.enable_dev = enable_dev,
};
