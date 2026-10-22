/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NORTHBRIDGE_OPTI_VIPERM_H
#define NORTHBRIDGE_OPTI_VIPERM_H

#include <arch/io.h>
#include <stdint.h>

/* 82C557M system registers: index to port 22h, then data at 24h (the index resets after each access). */
#define VIPERM_INDEX		0x22
#define VIPERM_DATA		0x24

#define VIPERM_MEM_DECODE_1	0x13	/* banks 0 (bits 2:0) and 1 (6:4) */
#define VIPERM_MEM_DECODE_2	0x14	/* banks 2 and 3 */
#define VIPERM_MEM_DECODE_3	0x19	/* banks 4 and 5 */

static inline u8 viperm_read(u8 index)
{
	outb(index, VIPERM_INDEX);
	return inb(VIPERM_DATA);
}

static inline void viperm_write(u8 index, u8 value)
{
	outb(index, VIPERM_INDEX);
	outb(value, VIPERM_DATA);
}

#endif
