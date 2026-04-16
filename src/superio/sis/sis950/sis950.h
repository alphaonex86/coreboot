/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SUPERIO_SIS_SIS950_H
#define SUPERIO_SIS_SIS950_H

/* SIS 950 Logical Device Numbers */
#define SIS950_FDC		0	/* Floppy Disk Controller */
#define SIS950_COM1		4	/* Serial Port 1 */
#define SIS950_COM2		3	/* Serial Port 2 */
#define SIS950_LPT		3	/* Parallel Port (shared LDN with COM2) */
#define SIS950_KBC		5	/* Keyboard Controller */

/* PnP Configuration port (default) */
#define SIS950_PNP_PORT		0x2E

/* PnP config mode unlock sequence */
#define SIS950_UNLOCK_1		0x87
#define SIS950_UNLOCK_2		0x01
#define SIS950_UNLOCK_3		0x55
#define SIS950_UNLOCK_4		0x55

/* PnP registers */
#define SIS950_PNP_LDN		0x07	/* Logical Device Number */
#define SIS950_PNP_ACTIVATE	0x30	/* Activate register */
#define SIS950_PNP_CLOCK	0x24	/* Clock settings */
#define SIS950_PNP_IOBASE_HI	0x60	/* I/O base address high byte */
#define SIS950_PNP_IOBASE_LO	0x61	/* I/O base address low byte */
#define SIS950_PNP_IRQ		0x70	/* Primary IRQ */
#define SIS950_PNP_DRQ		0x74	/* Primary DRQ */
#define SIS950_PNP_CONFIG	0x02	/* Configure control */

#include <device/pnp_type.h>
void sis950_enable_serial(pnp_devfn_t dev, u16 iobase);

#endif /* SUPERIO_SIS_SIS950_H */
