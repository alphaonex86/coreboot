/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <arch/romstage.h>
#include <cbmem.h>
#include <commonlib/helpers.h>
#include <console/console.h>
#include <cpu/486/newbios_post.h>
#include <device/pci_ops.h>
#include <southbridge/opti/82c558m/82c558m.h>

#include "viperm.h"

/* The IBM BIOS's 82C557M values (POST 86h) without L2 cache; DRAM sizing and timing apart. */
static const u8 init_table[][2] = {
	{ 0x00, 0x07 },	/* byte merge time-out 16 CLK */
	{ 0x02, 0x31 },	/* L2 off, write-back policy, CAS precharge 1 CLK */
	{ 0x03, 0x9a },	/* L2 timings */
	{ 0x08, 0x28 },	/* HITM# sampled without delay, tag and dirty in one SRAM */
	{ 0x0d, 0x02 },	/* the 82C557M claims the cycle after STOP# in PCI pre-snoop */
	{ 0x0e, 0x89 },	/* PCI master burst wait states, no parity check for masters */
	{ 0x0f, 0x80 },	/* PCI pre-snoop */
	{ 0x10, 0xe3 },	/* early PCI/VL/AT decode, 2-X-X-X pipelined reads, LCLK synchronous */
	{ 0x11, 0x20 },	/* SRAM chip select off when idle */
	{ 0x12, 0x02 },	/* refresh on each REFRESH# of the 82C558M */
	{ 0x15, 0xa0 },	/* CPU to PCI posting */
	{ 0x16, 0xa0 },	/* 7-bit tag plus dirty */
	{ 0x1d, 0x24 },	/* PEN# pin is MPERR# */
};

void __weak newbios_chipset_options(void)
{
}

void mainboard_romstage_entry(void)
{
	const unsigned int bus_mhz = mainboard_bus_mhz();

	post_code(POST_VIPERM_ROMSTAGE);
	for (int i = 0; i < ARRAY_SIZE(init_table); i++)
		viperm_write(init_table[i][0], init_table[i][1]);
	/* RAS# precharge and refresh pulse 5/6 CLK at 60 and 66 MHz, else 4/4 CLK (IBM: 50 MHz or unknown) */
	viperm_write(VIPERM_DRAM_CTL_1, bus_mhz >= 60 ? 0xdd : 0xfe);
	opti_82c558m_early_init(bus_mhz);

	viperm_dram_init();

	/* The IBM BIOS's last DRAM settings (POST 6Fh): posted writes, CPU address pipelining, fast BRDY# on
	   write page hits, hidden refresh */
	viperm_or(VIPERM_CACHE_CTL_1, 0x02);
	viperm_or(VIPERM_CPU_CACHE, 0x04);
	viperm_write(VIPERM_EXT_DMA, 0x40);
	pci_or_config8(ISA_BRIDGE, CYCLE_CTL, HIDDEN_REFRESH);
	newbios_chipset_options();

	cbmem_initialize_empty();
}
