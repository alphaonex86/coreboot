/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SOUTHBRIDGE_SIS_SIS85C503_H
#define SOUTHBRIDGE_SIS_SIS85C503_H

/* PCI Device IDs (Vendor 0x1039 - SIS) */
#define SIS85C503_DEVID		0x0008	/* ISA/LPC Bridge */
#define SIS5513_DEVID		0x5513	/* IDE Controller */
#define SIS7001_DEVID		0x7001	/* USB OHCI Controller */
#define SIS7018_DEVID		0x7018	/* Audio Controller */
#define SIS0900_DEVID		0x0900	/* Ethernet Controller */

/* ISA Bridge (SIS 503) PCI config registers */
#define SIS503_NVRAM_CTRL	0x40	/* NVRAM/Flash decode control */
#define SIS503_MISC_CTRL	0x45	/* Flash write enable etc. */
#define SIS503_KBC_CTRL		0x47	/* Keyboard/mouse controller */
#define SIS503_APC_CTRL		0x48	/* APC (GPIO) control */
#define SIS503_RTC_CTRL		0x48	/* RTC enable (shared with APC) */
#define SIS503_SIRQ_CTRL	0x70	/* Serial IRQ control */
#define SIS503_IRQ_ENABLE1	0x72	/* IRQ7-1 enable */
#define SIS503_IRQ_ENABLE2	0x73	/* IRQ15-8 enable */
#define SIS503_ACPI_BASE	0x74	/* ACPI base address */

/* NVRAM control register values */
#define NVRAM_DECODE_ENABLE	0x33	/* Enable FFF80000-FFFFFFFF decode */
#define FLASH_WRITE_ENABLE	0x40	/* Flash can be written */

/* Keyboard/Mouse controller */
#define KBC_MOUSE_ENABLE	0x0C	/* Enable keyboard + mouse */

/* Serial IRQ */
#define SIRQ_ENABLE		0x80	/* Enable serial IRQ */
#define IRQ_ALL_LOW		0xFD	/* All IRQs 7-1 enabled */
#define IRQ_ALL_HIGH		0xFF	/* All IRQs 15-8 enabled */

/* IDE Controller (SIS 5513) PCI config registers */
#define IDE_CMD_REG		0x04	/* PCI Command */
#define IDE_STATUS_REG		0x06	/* PCI Status */
#define IDE_PI_REG		0x09	/* Programming Interface */
#define IDE_LATENCY		0x0D	/* Latency Timer */
#define IDE_IRQ			0x3C	/* Interrupt Line */
#define IDE_PRIMARY_TIMING	0x40	/* Primary master timing */
#define IDE_PRIMARY_ACTIVE	0x41	/* Primary master active */
#define IDE_FIFO_THRESHOLD	0x48	/* FIFO threshold */
#define IDE_BM_CTRL		0x4A	/* Bus master control */
#define IDE_PREFETCH		0x4B	/* Prefetch control */
#define IDE_PRI_PREFETCH_CNT	0x4C	/* Primary prefetch count */
#define IDE_SEC_PREFETCH_CNT	0x4E	/* Secondary prefetch count */
#define IDE_MISC_CTRL		0x52	/* Miscellaneous control */

/* ACPI register offsets from ACPI base */
#define ACPI_SOFT_WDT		0x4B	/* Software watchdog */
#define ACPI_RESET_CTRL		0x56	/* Reset control */

/* 8254 Timer ports */
#define TIMER_CTRL		0x43
#define TIMER0_DATA		0x40

/* RTC ports */
#define RTC_INDEX		0x70
#define RTC_DATA		0x71
#define RTC_REG_A		0x0A
#define RTC_REG_B		0x0B
#define RTC_REG_D		0x0D

#endif /* SOUTHBRIDGE_SIS_SIS85C503_H */
