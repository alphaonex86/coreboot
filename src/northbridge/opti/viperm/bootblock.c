/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/bootblock.h>

#include "viperm.h"

/* E0000-FFFFF from the flash again (romstage runs at E0000): a CPU-only reset keeps the shadow RAM of the last boot. */
void bootblock_early_northbridge_init(void)
{
	viperm_write(VIPERM_SHADOW_3, 0x00);
}
