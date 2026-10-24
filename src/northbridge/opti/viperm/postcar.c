/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cpu/486/newbios_post.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <southbridge/opti/82c558m/82c558m.h>

#include "viperm.h"

/*
 * Cache-as-RAM is gone and the L1 on, still write-through (ramstage sets the policy).
 * C0000-FFFFF read and written in DRAM, for SeaBIOS and the option ROMs, C0000 and F0000 cacheable: only now,
 * as romstage runs from the flash at E0000. The flash stays at 4 GiB only.
 */
void late_car_teardown(void)
{
	post_code(POST_VIPERM_SHADOW);
	viperm_write(VIPERM_SHADOW_1, 0xf3);
	viperm_write(VIPERM_SHADOW_2, 0xff);
	viperm_write(VIPERM_SHADOW_3, 0x3f);
	pci_write_config8(ISA_BRIDGE, ROMCS_LOW, 0xc0);
}
