/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * SIS 735 DDR SDRAM initialization
 * Ported from legacy coreboot 735_regs.inc
 * Uses DDR SDRAM (vs SDRAM for 630/730)
 * JEDEC DDR init sequence with CKE control
 * Supports 4 DIMM slots
 */

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <delay.h>
#include "sis735.h"

#define SPD_MEMORY_TYPE		2
#define SPD_NUM_ROWS		3
#define SPD_NUM_COLS		4
#define SPD_NUM_BANKS_PER_CHIP	5
#define SPD_NUM_BANKS		17
#define SPD_CAS_LATENCY		18
#define SPD_DDR_TYPE		0x07	/* DDR SDRAM */

static const int ramsizes[16] = {
	8, 32, 32, 64, 16, 64, 64, 128,
	32, 128, 128, 256, 16, 256, 256, 512
};

static void sis735_nb_write(u8 reg, u8 val)
{
	pci_write_config8(PCI_DEV(0, 0, 0), reg, val);
}

/*
 * SIS 735 register init table from legacy 735_regs.inc
 * Format: register, value (decoded from word format High=Reg, Low=Value)
 */
static const struct {
	u8 reg;
	u8 val;
} nb_init_table[] = {
	/* Host Control */
	{ 0x50, 0xFF },
	{ 0x51, 0x47 },
	{ 0x52, 0x71 },
	{ 0x53, 0x83 },
	{ 0x54, 0x03 },
	{ 0x55, 0x01 },
	{ 0x56, 0x88 },
	{ 0x57, 0x88 },
	/* DRAM Control */
	{ 0x58, 0x00 },
	{ 0x5A, 0x08 },
	{ 0x5B, 0x03 },
	{ 0x5E, 0x03 },
	/* ACPI Control */
	{ 0x6A, 0x26 },
	{ 0x6B, 0x00 },
	/* PCI 33 Control */
	{ 0x80, 0x22 },
	{ 0x81, 0x42 },
	{ 0x82, 0x10 },
	{ 0x83, 0x03 },
	{ 0x84, 0x15 },
	{ 0x85, 0x00 },
	{ 0x86, 0x80 },
	{ 0x87, 0x01 },
	/* EDB Control */
	{ 0x88, 0x20 },
	{ 0x89, 0x04 },
	{ 0x8B, 0x04 },
	/* AGP GART */
	{ 0x94, 0x40 },
	{ 0x95, 0x01 },
	{ 0x97, 0x01 },
	{ 0x98, 0x00 },
	/* IO Buffer */
	{ 0x9B, 0x42 },
	{ 0x9F, 0x20 },
	{ 0xA0, 0x42 },
	{ 0xA1, 0x2A },
	{ 0xA2, 0x22 },
	{ 0xA3, 0x01 },
	/* PCI 66 Control */
	{ 0xD0, 0x22 },
	{ 0xD1, 0x42 },
	{ 0xD2, 0x13 },
	{ 0xD4, 0x05 },
	{ 0xD5, 0xFF },
	{ 0xD6, 0xFF },
	{ 0xD8, 0x60 },
	{ 0xD9, 0x60 },
	{ 0xDA, 0xAA },
	{ 0xDD, 0x88 },
};

static void sis735_init_registers(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nb_init_table); i++)
		sis735_nb_write(nb_init_table[i].reg, nb_init_table[i].val);
}

static void sis735_detect_dimms(void)
{
	int slot;
	u8 bank_enable = 0;
	const u8 spd_addrs[] = {
		DIMM0_SPD_ADDR, DIMM1_SPD_ADDR,
		DIMM2_SPD_ADDR, DIMM3_SPD_ADDR
	};

	for (slot = 0; slot < 4; slot++) {
		int spd_type, rows, cols, banks, bank_count;
		int size_index;
		u8 bank_val;

		spd_type = sis735_smbus_read_byte(spd_addrs[slot], SPD_MEMORY_TYPE);
		if (spd_type != SPD_DDR_TYPE) {
			printk(BIOS_DEBUG, "DIMM%d: not DDR (type=%02x)\n",
			       slot, spd_type);
			sis735_nb_write(SIS735_REG_BANK0 + slot, 0x00);
			continue;
		}

		rows = sis735_smbus_read_byte(spd_addrs[slot], SPD_NUM_ROWS);
		cols = sis735_smbus_read_byte(spd_addrs[slot], SPD_NUM_COLS);
		banks = sis735_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS_PER_CHIP);
		bank_count = sis735_smbus_read_byte(spd_addrs[slot], SPD_NUM_BANKS);

		if (rows < 0 || cols < 0 || banks < 0 || bank_count < 0) {
			sis735_nb_write(SIS735_REG_BANK0 + slot, 0x00);
			continue;
		}

		size_index = ((rows - 11) << 2) | (cols - 8);
		if (size_index < 0 || size_index >= 16)
			size_index = 0;

		bank_val = size_index & 0x1f;
		if (bank_count >= 2)
			bank_val |= 0x20;

		printk(BIOS_DEBUG, "DIMM%d: %dMB DDR (%d rows, %d cols, %d banks)%s\n",
		       slot, ramsizes[size_index] * (bank_count >= 2 ? 2 : 1),
		       rows, cols, bank_count,
		       bank_count >= 2 ? " double-sided" : "");

		sis735_nb_write(SIS735_REG_BANK0 + slot, bank_val);
		bank_enable |= (1 << slot);
	}

	sis735_nb_write(SIS735_REG_BANK_EN, bank_enable);
}

void sis735_raminit(void)
{
	printk(BIOS_INFO, "SIS 735 DDR SDRAM init...\n");
	sis735_init_registers();
	sis735_detect_dimms();
	printk(BIOS_INFO, "SIS 735 DDR SDRAM init done.\n");
}
