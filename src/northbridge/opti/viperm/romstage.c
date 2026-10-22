/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <arch/romstage.h>
#include <console/console.h>
#include <cpu/486/newbios_post.h>
#include <halt.h>

/* Port step 3 skeleton: romstage is reached from cache-as-RAM; DRAM init comes with step 4. */
void mainboard_romstage_entry(void)
{
	post_code(POST_VIPERM_ROMSTAGE);
	halt();
}
