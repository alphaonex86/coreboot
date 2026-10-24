/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NORTHBRIDGE_OPTI_VIPERM_H
#define NORTHBRIDGE_OPTI_VIPERM_H

#include <arch/io.h>
#include <stdint.h>

/* 82C557M system registers: index to port 22h, then data at 24h (the index resets after each access). */
#define VIPERM_INDEX		0x22
#define VIPERM_DATA		0x24

#define VIPERM_DRAM_CTL_1	0x01	/* DRAM timing */
#define VIPERM_CACHE_CTL_1	0x02
#define VIPERM_SHADOW_1		0x04	/* C0000-CFFFF, 2 bits per segment: bit 0 read DRAM, bit 1 write DRAM */
#define VIPERM_SHADOW_2		0x05	/* D0000-DFFFF */
#define VIPERM_SHADOW_3		0x06	/* E0000-FFFFF; bits 5, 4: C0000, F0000 cacheable */
#define VIPERM_CPU_CACHE	0x08
#define VIPERM_EXT_DMA		0x0c
#define VIPERM_MEM_DECODE_1	0x13	/* banks 0 (bits 2:0) and 1 (6:4) */
#define VIPERM_MEM_DECODE_2	0x14	/* banks 2 and 3 */
#define VIPERM_MEM_DECODE_3	0x19	/* banks 4 and 5 */

#define VIPERM_L1_WRITE_BACK	0x02	/* in VIPERM_CPU_CACHE */

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

static inline void viperm_or(u8 index, u8 bits)
{
	viperm_write(index, viperm_read(index) | bits);
}

/* The bus clock in MHz from the board's jumper (50, 60 or 66), 0 if unknown. */
unsigned int mainboard_bus_mhz(void);

void viperm_dram_init(void);

/* The mainboard's setup values for the chipset, applied by romstage after the DRAM init */
void newbios_chipset_options(void);
int viperm_l1_write_back(void);

#endif
