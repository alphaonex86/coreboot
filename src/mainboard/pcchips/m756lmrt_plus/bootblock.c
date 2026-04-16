/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <device/pnp_def.h>
#include <superio/ite/common/ite.h>
#include <superio/ite/it8705f/it8705f.h>

#define SERIAL_DEV PNP_DEV(0x2e, IT8705F_SP1)

void bootblock_mainboard_early_init(void)
{
	ite_enable_serial(SERIAL_DEV, CONFIG_TTYS0_BASE);
}
