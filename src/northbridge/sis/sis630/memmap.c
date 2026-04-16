/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/romstage.h>
#include <cbmem.h>
#include <commonlib/helpers.h>
#include <cpu/x86/mtrr.h>
#include <device/pci_ops.h>
#include <program_loading.h>
#include "sis630.h"

/*
 * Determine the top of usable DRAM.
 * RAM size is determined from the bank registers and SMA (shared memory area).
 */

static const int ramsizes[16] = {
	8, 32, 32, 64, 16, 64, 64, 128,
	32, 128, 128, 256, 16, 256, 256, 512
};

uintptr_t cbmem_top_chipset(void)
{
	pci_devfn_t nb_dev = PCI_DEV(0, 0, 0);
	int total_mb = 0;
	int slot;
	u8 bank_en, sma_size_reg;
	int sma_mb = 0;

	bank_en = pci_s_read_config8(nb_dev, SIS630_REG_BANK_EN);

	for (slot = 0; slot < 3; slot++) {
		u8 bank_val;
		int size_idx, banks;

		if (!(bank_en & (1 << slot)))
			continue;

		bank_val = pci_s_read_config8(nb_dev, SIS630_REG_BANK0 + slot);
		size_idx = bank_val & 0x1f;
		if (size_idx >= 16)
			size_idx = 0;
		banks = (bank_val & 0x20) ? 2 : 1;

		total_mb += ramsizes[size_idx] * banks;
	}

	/* Subtract SMA (shared memory area for VGA) */
	sma_size_reg = pci_s_read_config8(nb_dev, SIS630_REG_BANK_EN);
	if (sma_size_reg & 0x80) {
		int sma_bits = (sma_size_reg >> 4) & 0x7;
		sma_mb = 2 << sma_bits;
	}

	total_mb -= sma_mb;
	if (total_mb <= 0)
		total_mb = 8;

	return (uintptr_t)total_mb * 1024 * 1024;
}

void fill_postcar_frame(struct postcar_frame *pcf)
{
	const uintptr_t top_of_ram = cbmem_top();
	postcar_frame_add_mtrr(pcf, top_of_ram - 8 * MiB, 8 * MiB,
			       MTRR_TYPE_WRBACK);
}
