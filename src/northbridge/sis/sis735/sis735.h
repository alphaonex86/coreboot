/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef NORTHBRIDGE_SIS_SIS735_H
#define NORTHBRIDGE_SIS_SIS735_H

/* SIS 735 PCI device ID */
#define SIS735_DEVID		0x0735

/* Host Control */
#define SIS735_REG_CTRL0	0x50
#define SIS735_REG_CTRL1	0x51
#define SIS735_REG_CTRL2	0x52
#define SIS735_REG_CTRL3	0x53
#define SIS735_REG_CTRL4	0x54
#define SIS735_REG_CTRL5	0x55
#define SIS735_REG_CTRL6	0x56
#define SIS735_REG_CTRL7	0x57

/* DRAM Control */
#define SIS735_REG_PREDRV0	0x58
#define SIS735_REG_PREDRV2	0x5A
#define SIS735_REG_PREDRV3	0x5B
#define SIS735_REG_DDR_CTRL	0x5E

/* DRAM Bank Registers - supports 4 DIMM slots */
#define SIS735_REG_BANK0	0x60
#define SIS735_REG_BANK1	0x61
#define SIS735_REG_BANK2	0x62
#define SIS735_REG_BANK3	0x63
#define SIS735_REG_BANK_EN	0x64

/* ACPI Control */
#define SIS735_REG_ACPI0	0x6A
#define SIS735_REG_ACPI1	0x6B

/* PCI 33 Control */
#define SIS735_REG_PCI33_0	0x80
#define SIS735_REG_PCI33_1	0x81
#define SIS735_REG_PCI33_2	0x82
#define SIS735_REG_PCI33_3	0x83
#define SIS735_REG_PCI33_4	0x84
#define SIS735_REG_PCI33_5	0x85
#define SIS735_REG_PCI33_6	0x86
#define SIS735_REG_PCI33_7	0x87

/* EDB Control */
#define SIS735_REG_EDB0		0x88
#define SIS735_REG_EDB1		0x89
#define SIS735_REG_EDB3		0x8B

/* AGP GART */
#define SIS735_REG_GFX_WIN	0x94
#define SIS735_REG_GFX_PT	0x95
#define SIS735_REG_PT_CACHE	0x97
#define SIS735_REG_GFX_CTRL	0x98

/* IO Buffer */
#define SIS735_REG_IOBUF0	0x9B
#define SIS735_REG_IOBUF1	0x9F
#define SIS735_REG_IOBUF2	0xA0
#define SIS735_REG_IOBUF3	0xA1
#define SIS735_REG_IOBUF4	0xA2
#define SIS735_REG_IOBUF5	0xA3

/* PCI 66 Control */
#define SIS735_REG_PCI66_0	0xD0
#define SIS735_REG_PCI66_1	0xD1
#define SIS735_REG_PCI66_2	0xD2

/* SMBus base address */
#define SMB_BASE_ADDR		0x5080

/* SPD addresses for DIMM slots (4 slots) */
#define DIMM0_SPD_ADDR		0x50
#define DIMM1_SPD_ADDR		0x51
#define DIMM2_SPD_ADDR		0x52
#define DIMM3_SPD_ADDR		0x53

#ifndef __ASSEMBLER__
#include <types.h>

void sis735_early_smbus_init(void);
int sis735_smbus_read_byte(u8 device, u8 address);
void sis735_raminit(void);
#endif

#endif /* NORTHBRIDGE_SIS_SIS735_H */
