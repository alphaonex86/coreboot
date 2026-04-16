/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include "sis730.h"

void mainboard_romstage_entry(void)
{
	sis730_early_smbus_init();
	sis730_raminit();
	cbmem_initialize_empty();
}
