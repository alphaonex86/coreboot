/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include "sis630.h"

void mainboard_romstage_entry(void)
{
	/* Initialize SMBus for SPD reads */
	sis630_early_smbus_init();

	/* Initialize SDRAM */
	sis630_raminit();

	cbmem_initialize_empty();
}
