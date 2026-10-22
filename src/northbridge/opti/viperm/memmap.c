/* SPDX-License-Identifier: GPL-2.0-only */

#include <cbmem.h>
#include <commonlib/helpers.h>

#include "viperm.h"

/* Bank size from its 3-bit memory decode code: 0 none, 1-5 = 256Kx36 to 4Mx36, i.e. 2 to 32 MiB. */
static unsigned int bank_mib(u8 code)
{
	return code ? 1 << code : 0;
}

/* The DRAM end as raminit decoded it: the banks follow each other from 0, with no hole. */
uintptr_t cbmem_top_chipset(void)
{
	static const u8 decode_regs[] = {
		VIPERM_MEM_DECODE_1, VIPERM_MEM_DECODE_2, VIPERM_MEM_DECODE_3
	};
	unsigned int mib = 0;

	for (int i = 0; i < ARRAY_SIZE(decode_regs); i++) {
		const u8 banks = viperm_read(decode_regs[i]);
		mib += bank_mib(banks & 7) + bank_mib((banks >> 4) & 7);
	}
	return mib * MiB;
}
