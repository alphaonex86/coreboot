/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NORTHBRIDGE_SIS_SIS630_H
#define NORTHBRIDGE_SIS_SIS630_H

/* SIS 630 PCI device ID */
#define SIS630_DEVID		0x0630

/* Northbridge PCI registers */

/* Host Control Interface */
#define SIS630_REG_PIPELINE	0x50	/* CPU Pipeline control */
#define SIS630_REG_CTRL1	0x51

/* DRAM Control */
#define SIS630_REG_DRAM_CTRL0	0x52
#define SIS630_REG_DRAM_CTRL1	0x53
#define SIS630_REG_DRAM_FREQ0	0x54	/* 0x00=66/100MHz, 0x08=133MHz */
#define SIS630_REG_DRAM_FREQ1	0x55
#define SIS630_REG_DRAM_FREQ2	0x56	/* 0x00=66MHz, 0x80=100/133MHz */
#define SIS630_REG_DRAM_FREQ3	0x57	/* 0x00=100MHz, 0x01=133MHz */

/* Pre-driver Slew Rate / Current Driving */
#define SIS630_REG_PREDRV0	0x58
#define SIS630_REG_PREDRV1	0x59
#define SIS630_REG_PREDRV2	0x5A
#define SIS630_REG_PREDRV3	0x5B

/* DRAM Bank Registers */
#define SIS630_REG_BANK0	0x60	/* DRAM type/size bank 0 */
#define SIS630_REG_BANK1	0x61	/* DRAM type/size bank 1 */
#define SIS630_REG_BANK2	0x62	/* DRAM type/size bank 2 */
#define SIS630_REG_BANK_EN	0x63	/* Bank enable / SMA size */
#define SIS630_REG_SMA_LOC	0x65	/* DIMM location for SMA */

/* PCI Interface */
#define SIS630_REG_PCI0		0x80
#define SIS630_REG_PCI1		0x81
#define SIS630_REG_PCI2		0x82
#define SIS630_REG_PCI3		0x83
#define SIS630_REG_PCI4		0x84
#define SIS630_REG_PCI5		0x85
#define SIS630_REG_PCI6		0x86
#define SIS630_REG_PCI7		0x87
#define SIS630_REG_PCI8		0x88
#define SIS630_REG_PCI9		0x89

/* AGP GART / Graphics */
#define SIS630_REG_GART_BASE0	0x90
#define SIS630_REG_GFX_WIN	0x94	/* Graphics window size (0x40=64MB) */
#define SIS630_REG_PT_CACHE	0x97	/* Page table cache enable */
#define SIS630_REG_VGA_EN	0x9C	/* Integrated VGA enable */

/* DRAM Priority Timer */
#define SIS630_REG_PRIO0	0xA0
#define SIS630_REG_PRIO1	0xA1
#define SIS630_REG_PRIO2	0xA2
#define SIS630_REG_PRIO3	0xA3

/* AGP Command */
#define SIS630_REG_AGP_CMD0	0xC8
#define SIS630_REG_AGP_CMD1	0xC9

/* SMBus base address for SPD reads */
#define SMB_BASE_ADDR		0x5080

/* DIMM RAM size lookup table */
/* Index into ramsizes[] is bits [4:0] of bank register */
/* Bit 5: 1=double-sided, 0=single-sided */

/* SPD addresses for DIMM slots (on SMBus) */
#define DIMM0_SPD_ADDR		0x50
#define DIMM1_SPD_ADDR		0x51
#define DIMM2_SPD_ADDR		0x52

#ifndef __ASSEMBLER__
#include <types.h>

void sis630_early_smbus_init(void);
int sis630_smbus_read_byte(u8 device, u8 address);
void sis630_raminit(void);
#endif

#endif /* NORTHBRIDGE_SIS_SIS630_H */
