/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * x86 reset vector: the first instruction the CPU executes.
 *
 * After power-on or a hardware reset the 486 runs in real mode at physical
 * address 0xFFFFFFF0: CS selector 0xF000 but with a hidden base of 0xFFFF0000,
 * IP 0xFFF0. The chipset maps the top of the 4 GiB space to the flash chip,
 * and bootblock.ld places this .reset section exactly there (the last 16
 * bytes; the CBFS header pointer takes the last 4). There is only room for
 * one near jump, to _start16bit in entry16.S, which must therefore lie in the
 * same 64 KiB segment (0xFFFF0000-0xFFFFFFFF; bootblock.ld asserts it).
 */

	.section ".reset", "ax", %progbits
	.code16
.globl	_start
_start:
	/* 0xE9 = "jmp rel16" in 16-bit code. The displacement is relative to
	 * the end of the 3-byte instruction: "." is _start + 1 here, so
	 * ". + 2" is _start + 3. The CPU only reads the low 16 bits of the
	 * .int; its upper 2 bytes are never executed. */
	.byte  0xe9
	.int   _start16bit - ( . + 2 )
	/* Note: The above jump is hand coded to work around bugs in binutils.
	 * 5 byte are used for a 3 byte instruction.  This works because x86
	 * is little endian and allows us to use supported 32bit relocations
	 * instead of the weird 16 bit relocations that binutils does not
	 * handle consistently between versions because they are used so rarely.
	 */
