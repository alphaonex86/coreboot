/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * ROM identification block (data only, bootblock). bootblock.ld places the
 * .id section so that it ends at 0xFFFFFFF0, right below the reset vector.
 * Tools such as flashrom read it from the top of the image to identify the
 * board: three NUL-terminated strings, then for each string its address as
 * a negative offset from 4 GiB (0x100000000 - address, i.e. its distance
 * from the end of the address space), then the ROM size.
 * COREBOOT_VERSION comes from the generated build.h.
 */

#include <build.h>

.section ".id", "a", @progbits

ver:
	.asciz COREBOOT_VERSION
vendor:
	.asciz CONFIG_MAINBOARD_VENDOR
part:
	.asciz CONFIG_MAINBOARD_PART_NUMBER

#if ENV_X86_64 || defined(__clang__)
.long	0xffffffff - ver + 1		/* Reverse offset to the version */
.long	0xffffffff - vendor + 1		/* Reverse offset to the vendor id */
.long	0xffffffff - part + 1		/* Reverse offset to the part number */
#else
.long	- ver		/* Reverse offset to the version */
.long	- vendor	/* Reverse offset to the vendor id */
.long	- part		/* Reverse offset to the part number */
#endif

.long	CONFIG_ROM_SIZE /* Size of this romimage */
