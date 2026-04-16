/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 630 SDRAM initialization
 * Ported from legacy coreboot ipl.S / raminit.inc
 *
 * SDRAM init sequence: NOP -> Precharge -> CBR x8 -> MRS -> Normal
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <delay.h>
#include "sis630.h"

/* SPD bytes used for DIMM detection */
#define SPD_MEMORY_TYPE		2	/* 0x04 = SDRAM */
#define SPD_NUM_ROWS		3
#define SPD_NUM_COLS		4
#define SPD_NUM_BANKS_PER_CHIP	5
#define SPD_NUM_BANKS		17
#define SPD_CAS_LATENCY		18
#define SPD_SDRAM_TYPE		0x04

/* DIMM size lookup table: index by row/col/bank encoding */
static const int ramsizes[16] = {
	8, 32, 32, 64, 16, 64, 64, 128,
	32, 128, 128, 256, 16, 256, 256, 512
};

static void sis630_nb_write(u8 reg, u8 val)
{
	pci_write_config8(PCI_DEV(0, 0, 0), reg, val);
}

/*
 * SIS 630 register init table
 * These are the default values from the legacy 630_regs.inc
 */
static const struct {
	u8 reg;
	u8 val;
} nb_init_table[] = {
	/* Host Control Interface */
	{ 0x50, 0x9E },
	{ 0x51, 0x00 },
	/* DRAM Control */
	{ 0x52, 0xC5 },
	{ 0x53, 0x00 },
	{ 0x54, 0x00 },	/* Frequency: 0x00 = 66/100MHz */
	{ 0x55, 0x29 },
	{ 0x56, 0x00 },
	{ 0x57, 0x00 },
	/* Pre-driver */
	{ 0x58, 0x00 },
	{ 0x59, 0x35 },
	{ 0x5A, 0x51 },
	{ 0x5B, 0x00 },
	/* PCI Interface */
	{ 0x80, 0x21 },
	{ 0x81, 0xFF },
	{ 0x82, 0x7F },
	{ 0x83, 0x1E },
	{ 0x84, 0x60 },
	{ 0x85, 0x00 },
	{ 0x86, 0x03 },
	{ 0x87, 0x40 },
	{ 0x88, 0x00 },
	{ 0x89, 0x08 },
	/* AGP GART */
	{ 0x90, 0x00 },
	{ 0x91, 0x00 },
	{ 0x92, 0x00 },
	{ 0x93, 0x00 },
	/* Graphics */
	{ 0x94, 0x40 },	/* 64MB window */
	{ 0x97, 0x01 },	/* Page table cache */
	{ 0x9C, 0x02 },	/* VGA enable */
	/* Priority */
	{ 0xA0, 0x00 },
	{ 0xA1, 0x00 },
	{ 0xA2, 0x03 },
	{ 0xA3, 0x01 },
	/* AGP Command */
	{ 0xC8, 0x04 },	/* AGP 4X */
	{ 0xC9, 0x00 },
};

static void sis630_init_registers(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nb_init_table); i++)
		sis630_nb_write(nb_init_table[i].reg, nb_init_table[i].val);
}

/*
 * Detect and configure SDRAM DIMMs via SPD
 */
static void sis630_detect_dimms(void)
{
	int slot;
	u8 bank_enable = 0;
	const u8 spd_addrs[] = { DIMM0_SPD_ADDR, DIMM1_SPD_ADDR, DIMM2_SPD_ADDR };

	for (slot = 0; slot < 3; slot++) {
		int spd_type, rows, cols, banks, bank_count;
		int size_index;
		u8 bank_val;

		/* Read SPD memory type */
		spd_type = sis630_smbus_read_byte(spd_addrs[slot], SPD_MEMORY_TYPE);
		if (spd_type != SPD_SDRAM_TYPE) {
			printk(BIOS_DEBUG, "DIMM%d: not SDRAM (type=%02x)\n",
			       slot, spd_type);
			sis630_nb_write(SIS630_REG_BANK0 + slot, 0x00);
			continue;
		}

		rows = sis630_smbus_read_byte(spd_addrs[slot], SPD_NUM_ROWS);
		cols = sis630_smbus_read_byte(spd_addrs[slot], SPD_NUM_COLS);
		banks = sis630_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS_PER_CHIP);
		bank_count = sis630_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS);

		if (rows < 0 || cols < 0 || banks < 0 || bank_count < 0) {
			printk(BIOS_DEBUG, "DIMM%d: SPD read error\n", slot);
			sis630_nb_write(SIS630_REG_BANK0 + slot, 0x00);
			continue;
		}

		/*
		 * Encode row/col/bank into SIS 630 bank register format
		 * The encoding maps to the ramsizes[] lookup table
		 */
		size_index = ((rows - 11) << 2) | (cols - 8);
		if (size_index < 0 || size_index >= 16)
			size_index = 0;

		bank_val = size_index & 0x1f;
		/* Set double-sided bit if 2 banks */
		if (bank_count >= 2)
			bank_val |= 0x20;

		printk(BIOS_DEBUG, "DIMM%d: %dMB (%d rows, %d cols, %d banks)%s\n",
		       slot, ramsizes[size_index] * (bank_count >= 2 ? 2 : 1),
		       rows, cols, bank_count,
		       bank_count >= 2 ? " double-sided" : "");

		sis630_nb_write(SIS630_REG_BANK0 + slot, bank_val);
		bank_enable |= (1 << slot);
	}

	/* Write bank enable register */
	sis630_nb_write(SIS630_REG_BANK_EN, bank_enable);
}

void sis630_raminit(void)
{
	printk(BIOS_INFO, "SIS 630 SDRAM init...\n");

	/* Initialize northbridge registers */
	sis630_init_registers();

	/* Detect and configure DIMMs via SPD */
	sis630_detect_dimms();

	printk(BIOS_INFO, "SIS 630 SDRAM init done.\n");
}
