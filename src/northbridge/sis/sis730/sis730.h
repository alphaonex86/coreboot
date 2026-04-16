/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NORTHBRIDGE_SIS_SIS730_H
#define NORTHBRIDGE_SIS_SIS730_H

/* SIS 730 PCI device ID */
#define SIS730_DEVID		0x0730

/* Northbridge PCI registers */

/* Host Control Interface */
#define SIS730_REG_CTRL0	0x50
#define SIS730_REG_CTRL1	0x51

/* DRAM Control */
#define SIS730_REG_DRAM_CTRL0	0x52
#define SIS730_REG_DRAM_CTRL1	0x53
#define SIS730_REG_DRAM_FREQ0	0x54
#define SIS730_REG_DRAM_FREQ1	0x55
#define SIS730_REG_DRAM_FREQ3	0x57

/* Pre-driver Slew Rate */
#define SIS730_REG_PREDRV0	0x58
#define SIS730_REG_PREDRV1	0x59
#define SIS730_REG_PREDRV2	0x5A
#define SIS730_REG_PREDRV3	0x5B

/* DRAM Bank Registers */
#define SIS730_REG_BANK0	0x60
#define SIS730_REG_BANK1	0x61
#define SIS730_REG_BANK2	0x62
#define SIS730_REG_BANK_EN	0x63
#define SIS730_REG_SMA_LOC	0x65

/* Misc Control */
#define SIS730_REG_MISC0	0x6A
#define SIS730_REG_MISC1	0x6B
#define SIS730_REG_MISC2	0x6C

/* PCI Interface */
#define SIS730_REG_PCI0		0x80
#define SIS730_REG_PCI1		0x81
#define SIS730_REG_PCI2		0x82
#define SIS730_REG_PCI3		0x83
#define SIS730_REG_PCI4		0x84
#define SIS730_REG_PCI5		0x85
#define SIS730_REG_PCI6		0x86
#define SIS730_REG_PCI7		0x87
#define SIS730_REG_PCI8		0x88
#define SIS730_REG_PCI9		0x89
#define SIS730_REG_PCI10	0x8A
#define SIS730_REG_PCI11	0x8B

/* AGP / Graphics */
#define SIS730_REG_GFX_WIN	0x94
#define SIS730_REG_PT_CACHE	0x97

/* DRAM Priority */
#define SIS730_REG_PRIO0	0xA0
#define SIS730_REG_PRIO1	0xA1

/* SMBus base address for SPD reads */
#define SMB_BASE_ADDR		0x5080

/* SPD addresses for DIMM slots */
#define DIMM0_SPD_ADDR		0x50
#define DIMM1_SPD_ADDR		0x51
#define DIMM2_SPD_ADDR		0x52

#ifndef __ASSEMBLER__
#include <types.h>

void sis730_early_smbus_init(void);
int sis730_smbus_read_byte(u8 device, u8 address);
void sis730_raminit(void);
#endif

#endif /* NORTHBRIDGE_SIS_SIS730_H */
