/* SPDX-License-Identifier: GPL-2.0-only */

#include <cbmem.h>
#include <commonlib/helpers.h>
#include <console/console.h>
#include <cpu/486/newbios_post.h>
#include <device/mmio.h>
#include <halt.h>
#include <stdbool.h>

#include "viperm.h"

/*
 * DRAM sizing. Each SIMM pair (J23/J24: 13h, J27/J28: 14h) is two 64-bit banks, side A (bits 2:0) and side B
 * (bits 6:4); code n is a bank of 2 MiB << (n - 1) that drives 9 + n / 2 row and 9 + (n - 1) / 2 column MA
 * lines (data book table 4-16), the banks follow each other from 0. Each side is probed at code 7: an MA line
 * the SIMM lacks makes its address alias. It then gets the largest code its lines fill, at most 5 as MA11 is
 * RAS4# (19h = 88h): the codes of the IBM BIOS (boards/aptiva2168/test/simm_configs.txt in retro-firmware).
 * The probes stay out of the cache-as-RAM lines.
 */

#define MIN_DRAM	(2 * MiB)

static const u8 decode_regs[] = { VIPERM_MEM_DECODE_1, VIPERM_MEM_DECODE_2 };
static const u8 decode_init[] = { 0x88, 0x80 };	/* IBM: 13h full decode, SMRAM; 14h DLE1# */

static uintptr_t dram_top;

/* As the IBM BIOS after each decode change: 128 refresh toggles of port 61h bit 4, about 2 ms. */
static void decode_write(u8 reg, u8 value)
{
	u8 last = inb(0x61) & 0x10;

	viperm_write(reg, value);
	for (int toggles = 0, reads = 0; toggles < 128 && reads < 100000; reads++) {
		const u8 now = inb(0x61) & 0x10;
		toggles += now != last;
		last = now;
	}
}

/* The dword at a keeps its pattern when the complement goes to a + delta, at three offsets: one vote each,
   so that a faulty bit can turn only one. A floating bus returns the last value written: "no". */
static bool distinct(uintptr_t a, uintptr_t delta)
{
	static const u16 offsets[] = { 0x000, 0x48c, 0x918 };
	int votes = 0;

	for (int i = 0; i < ARRAY_SIZE(offsets); i++) {
		const uintptr_t p = a + offsets[i];
		const u32 pattern = p ^ 0x5aa5c33c;

		write32p(p, pattern);
		write32p(p + delta, ~pattern);
		votes += read32p(p) == pattern;
	}
	return votes >= 2;
}

/* The code for the side at base (0: none); *partial when the SIMM holds more than that code uses. */
static u8 side_code(uintptr_t base, bool *partial)
{
	int rows = 0, cols = 0;

	if (!distinct(base, 8))
		return 0;
	while (rows < 3 && distinct(base, 2 * MiB << (2 * rows)))	/* row MA9-11: A21, A23, A25 */
		rows++;
	while (cols < 3 && distinct(base, 4 * MiB << (2 * cols)))	/* column MA9-11: A22, A24, A26 */
		cols++;
	const u8 code = MIN(MIN(2 * rows + 1, 2 * cols + 2), 5);
	*partial |= rows + cols + 1 > code;
	return code;
}

static bool count_block(uintptr_t a)
{
	return (a < 0xa0000 || a >= 1 * MiB) && (a < CONFIG_DCACHE_RAM_BASE ||
		a >= CONFIG_DCACHE_RAM_BASE + CONFIG_DCACHE_RAM_SIZE);
}

/* The IBM BIOS's memory count: one dword per 64 KiB holds its address, then the complement; the DRAM ends at
   the MiB of the first that fails. */
static uintptr_t memory_count(uintptr_t top)
{
	uintptr_t end = top;

	for (u32 flip = 0; flip < 2; flip++) {
		for (uintptr_t a = 0; a < end; a += 64 * KiB)
			if (count_block(a))
				write32p(a, a ^ -flip);
		for (uintptr_t a = 0; a < end; a += 64 * KiB)
			if (count_block(a) && read32p(a) != (a ^ -flip))
				end = ALIGN_DOWN(a, 1 * MiB);
	}
	return end;
}

void viperm_dram_init(void)
{
	u8 code[4];
	bool partial = false;

	post_code(POST_VIPERM_DRAM);
	decode_write(VIPERM_MEM_DECODE_3, 0x80);	/* MA11 as MA11 while probing */
	for (int pair = 0; pair < 2; pair++) {
		decode_write(decode_regs[!pair], decode_init[!pair]);
		decode_write(decode_regs[pair], decode_init[pair] | 0x77);
		code[2 * pair] = side_code(0, &partial);
		code[2 * pair + 1] = side_code(128 * MiB, &partial);
	}
	decode_write(VIPERM_MEM_DECODE_1, decode_init[0] | code[1] << 4 | code[0]);
	decode_write(VIPERM_MEM_DECODE_2, decode_init[1] | code[3] << 4 | code[2]);
	decode_write(VIPERM_MEM_DECODE_3, 0x88);

	uintptr_t top = 0;
	for (int bank = 0; bank < ARRAY_SIZE(code); bank++)
		top += code[bank] ? 1 * MiB << code[bank] : 0;
	if (partial) {
		printk(BIOS_WARNING, "DRAM: a SIMM holds more than the 82C557M addresses, the rest is unused\n");
		post_code(POST_VIPERM_DRAM_PART);
	}
	dram_top = memory_count(top);
	printk(BIOS_INFO, "DRAM: SIMM bank codes %u %u %u %u, %lu KiB\n",
	       code[0], code[1], code[2], code[3], dram_top / KiB);
	if (dram_top < top) {
		printk(BIOS_WARNING, "DRAM: %lu KiB of %lu KiB, a 64 KiB block failed above\n",
		       dram_top / KiB, top / KiB);
		post_code(POST_VIPERM_DRAM_CUT);
	}
	if (dram_top < MIN_DRAM) {
		printk(BIOS_EMERG, "DRAM: none usable\n");
		post_code(POST_VIPERM_NO_DRAM);
		halt();
	}
}

uintptr_t cbmem_top_chipset(void)
{
	return dram_top;
}
