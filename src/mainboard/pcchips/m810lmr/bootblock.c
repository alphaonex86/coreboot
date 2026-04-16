/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <device/pnp_def.h>
#include <superio/sis/sis950/sis950.h>

#define SERIAL_DEV PNP_DEV(0x2e, SIS950_COM1)

void bootblock_mainboard_early_init(void)
{
	sis950_enable_serial(SERIAL_DEV, 0x3f8);
}
