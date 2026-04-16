/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 730 SDRAM initialization
 * Ported from legacy coreboot 730s_regs.inc
 * Supports Socket A (K7) and Socket 370
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <delay.h>
#include "sis730.h"

#define SPD_MEMORY_TYPE		2
#define SPD_NUM_ROWS		3
#define SPD_NUM_COLS		4
#define SPD_NUM_BANKS_PER_CHIP	5
#define SPD_NUM_BANKS		17
#define SPD_SDRAM_TYPE		0x04

static const int ramsizes[16] = {
	8, 32, 32, 64, 16, 64, 64, 128,
	32, 128, 128, 256, 16, 256, 256, 512
};

static void sis730_nb_write(u8 reg, u8 val)
{
	pci_write_config8(PCI_DEV(0, 0, 0), reg, val);
}

/*
 * SIS 730S register init table from legacy 730s_regs.inc
 */
static const struct {
	u8 reg;
	u8 val;
} nb_init_table[] = {
	{ 0x50, 0xC5 },
	{ 0x51, 0x10 },
	{ 0x52, 0x80 },
	{ 0x53, 0x00 },
	{ 0x54, 0x00 },
	{ 0x55, 0x01 },
	{ 0x57, 0x00 },
	{ 0x58, 0x00 },
	{ 0x59, 0x05 },
	{ 0x5A, 0x55 },
	{ 0x5B, 0x00 },
	{ 0x6A, 0x66 },
	{ 0x6B, 0x1E },
	{ 0x6C, 0x03 },
	{ 0x80, 0x72 },
	{ 0x81, 0x07 },
	{ 0x82, 0xFF },
	{ 0x83, 0xFF },
	{ 0x84, 0x83 },
	{ 0x85, 0x24 },
	{ 0x86, 0x00 },
	{ 0x87, 0x00 },
	{ 0x88, 0xF0 },
	{ 0x89, 0xF0 },
	{ 0x8A, 0xF0 },
	{ 0x8B, 0xF0 },
	{ 0x94, 0x40 },
	{ 0xA0, 0x23 },
	{ 0xA1, 0x43 },
};

static void sis730_init_registers(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nb_init_table); i++)
		sis730_nb_write(nb_init_table[i].reg, nb_init_table[i].val);
}

static void sis730_detect_dimms(void)
{
	int slot;
	u8 bank_enable = 0;
	const u8 spd_addrs[] = { DIMM0_SPD_ADDR, DIMM1_SPD_ADDR, DIMM2_SPD_ADDR };

	for (slot = 0; slot < 3; slot++) {
		int spd_type, rows, cols, banks, bank_count;
		int size_index;
		u8 bank_val;

		spd_type = sis730_smbus_read_byte(spd_addrs[slot], SPD_MEMORY_TYPE);
		if (spd_type != SPD_SDRAM_TYPE) {
			printk(BIOS_DEBUG, "DIMM%d: not SDRAM (type=%02x)\n",
			       slot, spd_type);
			sis730_nb_write(SIS730_REG_BANK0 + slot, 0x00);
			continue;
		}

		rows = sis730_smbus_read_byte(spd_addrs[slot], SPD_NUM_ROWS);
		cols = sis730_smbus_read_byte(spd_addrs[slot], SPD_NUM_COLS);
		banks = sis730_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS_PER_CHIP);
		bank_count = sis730_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS);

		if (rows < 0 || cols < 0 || banks < 0 || bank_count < 0) {
			sis730_nb_write(SIS730_REG_BANK0 + slot, 0x00);
			continue;
		}

		size_index = ((rows - 11) << 2) | (cols - 8);
		if (size_index < 0 || size_index >= 16)
			size_index = 0;

		bank_val = size_index & 0x1f;
		if (bank_count >= 2)
			bank_val |= 0x20;

		printk(BIOS_DEBUG, "DIMM%d: %dMB (%d rows, %d cols, %d banks)%s\n",
		       slot, ramsizes[size_index] * (bank_count >= 2 ? 2 : 1),
		       rows, cols, bank_count,
		       bank_count >= 2 ? " double-sided" : "");

		sis730_nb_write(SIS730_REG_BANK0 + slot, bank_val);
		bank_enable |= (1 << slot);
	}

	sis730_nb_write(SIS730_REG_BANK_EN, bank_enable);
}

void sis730_raminit(void)
{
	printk(BIOS_INFO, "SIS 730 SDRAM init...\n");
	sis730_init_registers();
	sis730_detect_dimms();
	printk(BIOS_INFO, "SIS 730 SDRAM init done.\n");
}
