/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SOUTHBRIDGE_OPTI_82C558M_H
#define SOUTHBRIDGE_OPTI_82C558M_H

#define ISA_BRIDGE	PCI_DEV(0, 1, 0)

#define PIRQ_ROUTE	0x40	/* 16 bits: PIRQ0-3# to an ISA IRQ, 3 bits each */
#define MISC_1		0x42	/* bits 7:1: IRQ15, 14, 12, 11, 10, 9, 5 level-triggered */
#define CYCLE_CTL	0x47	/* bit 6 hidden refresh, bits 5:4 ATCLK divider */
#define ROMCS_LOW	0x4a	/* ROMCS# for E0000-FFFFF */
#define ROMCS_HIGH	0x4b	/* ROMCS# below 4 GiB, one bit per 32 KiB */

#define HIDDEN_REFRESH	0x40

void opti_82c558m_early_init(unsigned int bus_mhz);

#endif
