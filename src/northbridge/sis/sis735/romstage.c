/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <console/console.h>
#include "sis735.h"

void mainboard_romstage_entry(void)
{
	sis735_early_smbus_init();
	sis735_raminit();
	cbmem_initialize_empty();
}
