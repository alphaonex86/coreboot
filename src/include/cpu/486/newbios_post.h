/* SPDX-License-Identifier: GPL-2.0-only */

/* newBIOS POST codes on port 80h for the 486 bootblock, romstage and postcar (values not used by
   commonlib/console/post_codes.h, which covers ramstage). Only #defines: included by assembly.
   Meanings: newBIOS/POSTCODES.md. */
#ifndef CPU_486_NEWBIOS_POST_H
#define CPU_486_NEWBIOS_POST_H

#define POST_486_SOFT_RESET		0x02	/* CMOS shutdown byte set: CPU-only reset, jump to SeaBIOS */
#define POST_486_GDT			0x03	/* real mode: IDT and GDT loaded */
#define POST_486_PROTECTED_MODE		0x04	/* CR0.PE set, far jump to 32-bit code next */
#define POST_486_CAR_FILL		0x22	/* L1 on, reading the cache-as-RAM range into it */
#define POST_486_CAR_LOCKED		0x23	/* CR0.CD = NW = 1: the L1 lines are now the RAM */
#define POST_486_CAR_DRAM_OFF		0x26	/* temporary UM8881F DRAM setup removed */
#define POST_486_CAR_CLEAR		0x27	/* cache-as-RAM zeroed */
#define POST_486_CAR_STACK		0x28	/* stack in cache-as-RAM, calling the bootblock C code */
#define POST_486_BOOTBLOCK_C		0x14	/* bootblock C code, stack in cache-as-RAM */
#define POST_486_BIST_OK		0x15	/* CPU self test passed */
#define POST_486_EARLY_NORTHBRIDGE	0x16	/* UM8881F: ROM decode, shadow and caches off */
#define POST_486_EARLY_SOUTHBRIDGE	0x17	/* UM8886BF early setup */
#define POST_486_EARLY_CPU		0x18
#define POST_486_LOAD_ROMSTAGE		0x19	/* bootblock done, romstage next */

#define POST_UMC_HOST_BRIDGE		0x30	/* romstage: UM8881F register defaults */
#define POST_UMC_PCI_QUIESCE		0x31	/* PCI devices turned off before DRAM sizing */
#define POST_UMC_DRAM			0x32	/* DRAM banks sized (52h/53h/61h) */
#define POST_UMC_L2			0x33	/* L2 cache sized and enabled */
#define POST_UMC_SOUTHBRIDGE		0x35	/* UM8886BF early init */
#define POST_UMC_SETUP_VALUES		0x37	/* setup chipset values from CMOS */
#define POST_UMC_SHADOW			0x38	/* BIOS copied to shadow RAM E/F, C-F shadowed */
#define POST_UMC_CBMEM			0x3a	/* CBMEM at the top of RAM */
#define POST_UMC_LOAD_POSTCAR		0x3b	/* romstage done, postcar next */
#define POST_486_POSTCAR		0x3c	/* postcar, still on the cache-as-RAM stack */
#define POST_486_CAR_OFF		0x3d	/* cache-as-RAM left, L1 normal: ramstage next */

#endif
