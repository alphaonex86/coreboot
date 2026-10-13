/* SPDX-License-Identifier: GPL-2.0-or-later */

#define __SIMPLE_DEVICE__

#include <console/console.h>
#include <arch/romstage.h>
#include <cbmem.h>
#include <cpu/486/newbios_post.h>
#include <device/mmio.h>
#include <device/pci_def.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include <string.h>	//for memcpy

#include "reg_init.h"

/*
 * After a CPU-only reset (port 92h, keyboard controller) PCI cards keep decoding their BARs, and a
 * memory BAR above the RAM (e.g. a VGA framebuffer) would be sized as DRAM. Turn off all devices
 * except bridges and IDE (legacy I/O only) before the DRAM probe; ramstage enables them again.
 */
void __weak newbios_chipset_options(void)
{
}

static void pci_quiesce_devices(void)
{
	for (unsigned int devfn = 0; devfn < 0x100; devfn++) {
		const pci_devfn_t dev = PCI_DEV(0, PCI_SLOT(devfn), PCI_FUNC(devfn));
		const u16 class = pci_read_config16(dev, PCI_CLASS_DEVICE);

		if (pci_read_config16(dev, PCI_VENDOR_ID) == 0xffff
				|| (class >> 8) == PCI_BASE_CLASS_BRIDGE || class == PCI_CLASS_STORAGE_IDE)
			continue;
		pci_write_config16(dev, PCI_COMMAND, 0);
	}
}


static const struct umc_cfg_entry hb_init_table[] = {
	//Auto Configuration = the original BIOS's values for a 33 MHz bus (its own table, verified in
	//86Box): L2 3-1-1-1, DRAM read and write 1 WS, no burst copy-back, no EDO. The faster 2-1-1-1,
	//0 WS read and EDO values used before froze benchmarks on the real board.
	//NOTICE wriback must be enabled at exit_car after filling
	{HB50_L2_CFG_0,			0x40 | 0x30,	0x10},

	{HB51_L2_CFG_1,			0xff,	0xa4},

	// {HB52_DRAM_CFG_0,		0,	0xe6},
	// {HB53_DRAM_CFG_1,		0,	0x8f},
	{HB52_DRAM_CFG_0,		0x00,	0x80},
	{HB53_DRAM_CFG_1,		0x40,	0x80},

	{HB54_SHADOW_EN,		0xff,	0x00},
	{HB55_SHADOW_CFG,		0xff,	0x40},	//readonly
	{HB56_MEMHOLE_BASE,		0xff,	0xff},
	{HB57_MEMHOLE_SIZE,		0xff,	0x0f},
	{HB58_UNKNOWN_0,		0xff,	0xff},
	{HB59_UNKNOWN_1,		0xff,	0x0f},
	{HB5A_MISC_0,			0xff,	0x0d},
	//5b unknown
	{HB5C_SMRAM_0,			0xff,	0xe0},
	{HB5D_SMRAM_1_DRAM_CFG_2,	0xff,	0x30},
	//5e-5f unknown, implemented
	{HB60_SMRAM_2,			0xff,	0x00},

	{HB61_EDO_CFG,			0xff,	0x04},
	{HB62_BURST,			0xff,	0x01},
};


/*
 * Fill the E and F segment shadow RAM like the original BIOS: from code in DRAM with the source in DRAM,
 * never writing E0000-FFFFF while running from the flash (romstage runs from the E segment).
 */
#define SHADOW_CODE	0x90000

extern const char shadow_fill_start[], shadow_fill_end[];

asm(".section .text.shadow_fill, \"ax\", @progbits\n"
    "shadow_fill_start:\n"
    "	pushl	%esi\n"
    "	pushl	%edi\n"
    "	cld\n"
    "	movl	$0xfffe0000, %esi\n"
    "	movl	$0x100000, %edi\n"
    "	movl	$0x8000, %ecx\n"
    "	rep movsl\n"
    "	movl	$0x100000, %esi\n"
    "	movl	$0xe0000, %edi\n"
    "	movl	$0x8000, %ecx\n"
    "	rep movsl\n"
    "	popl	%edi\n"
    "	popl	%esi\n"
    "	ret\n"
    "shadow_fill_end:\n"
    ".previous\n");

static void shadow_fill_ef(void)
{
	memcpy((void *)SHADOW_CODE, shadow_fill_start, shadow_fill_end - shadow_fill_start);
	((void (*)(void))SHADOW_CODE)();
}

void dram_init(void);
void l2_cache_init(void);
void um8886_early_init(void);


void mainboard_romstage_entry(void)
{
	printk(BIOS_DEBUG, "mainboard romstage\n");

	//init HB registers
	post_code(POST_UMC_HOST_BRIDGE);
	umc_registers_init(
		HOST_BRIDGE,
		hb_init_table,
		ARRAY_SIZE(hb_init_table)
	);
	//TODO make some configurable from CMOS

	post_code(POST_UMC_PCI_QUIESCE);
	pci_quiesce_devices();

	//detect dram, L2 disabled (no interference)
	post_code(POST_UMC_DRAM);
	dram_init();

	//L2 cache initialization
	post_code(POST_UMC_L2);
	l2_cache_init();

	//NOTICE L2 is configured and enabled now

	// asm volatile("rdtsc" :::);

	printk(BIOS_INFO, "RAM size %lu kiB\n", cbmem_top_chipset()/KiB);

	post_code(POST_UMC_SOUTHBRIDGE);
	um8886_early_init();

	post_code(POST_UMC_SETUP_VALUES);
	newbios_chipset_options();

/*
	printk(BIOS_DEBUG, "Erase high RAM\n");
	for (uintptr_t addr = 128*MiB - 1*MiB; addr < 128*MiB; addr+=4) {
		if ((addr % 0x10000) == 0)
			printk(BIOS_DEBUG, "[%08lx]\n",addr);
		write32p(addr, 0);
	}
*/

	//move raminit to bootblock?

	//disable write protect
	post_code(POST_UMC_SHADOW);
	pci_update_config8(HOST_BRIDGE, HB55_SHADOW_CFG, ~0x40, 0);

	//copy the flash (E and F) to shadow E and F
	shadow_fill_ef();


	//CSEG shadow
	pci_update_config8(HOST_BRIDGE, HB54_SHADOW_EN, ~0, 0xe);

	//DSEG shadow
	pci_update_config8(HOST_BRIDGE, HB54_SHADOW_EN, ~0, 0xf0);

	//segE shadow read
	pci_update_config8(HOST_BRIDGE, HB54_SHADOW_EN, ~0, 1);

	//global + segF shadow read
	pci_update_config8(HOST_BRIDGE, HB55_SHADOW_CFG, ~0, 0x80);


	//cmos_get_uint_option

	//NOTICE NOTICE NOTICE NOTICE experiments
	//set if pci video bios installed?
	pci_update_config8(ISA_BRIDGE, IBC50_PCI_VIDEO, ~0, 0x80);

	pci_update_config8(ISA_BRIDGE, IBC50_PCI_VIDEO, ~0, 1);

	pci_update_config8(ISA_BRIDGE, IBC51_MEM_SIZE, ~0, 1);
	//NOTICE NOTICE NOTICE NOTICE experiments


#if 0
	printk(BIOS_DEBUG, "dump HB:\n");
	for (unsigned idx=0x50;idx<0x63;idx++) {
		if ((idx % 16) == 0) {
			printk(BIOS_DEBUG, "%02x: ", idx);
		}
		printk(BIOS_DEBUG, "%02hhx ",
		       pci_read_config8(HOST_BRIDGE, idx)
		);
		if (((idx % 16) == 15) || (idx == (0x63-1))) {
			printk(BIOS_DEBUG, "\n");
		}
	}

	printk(BIOS_DEBUG, "dump IBC:\n");
	for (unsigned idx=0x40;idx<0xb0;idx++) {
		if ((idx % 16) == 0) {
			printk(BIOS_DEBUG, "%02x: ", idx);
		}
		printk(BIOS_DEBUG, "%02hhx ",
		       pci_read_config8(ISA_BRIDGE, idx)
		);
		if (((idx % 16) == 15) || (idx == (0xb0-1))) {
			printk(BIOS_DEBUG, "\n");
		}
	}
#endif


// printk(BIOS_DEBUG, "cbmem_initialize {\n");

	post_code(POST_UMC_CBMEM);
	cbmem_initialize_empty();

	printk(BIOS_DEBUG, "UMC HB romstage exit\n");
	post_code(POST_UMC_LOAD_POSTCAR);
}
