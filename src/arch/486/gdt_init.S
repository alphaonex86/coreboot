/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * The "ROM" GDT used from the reset vector until ramstage (c_start.S then
 * loads a larger GDT in RAM). It is linked into three stages:
 *  - bootblock: entry16.S loads gdtptr directly with lgdtl (gdt_init()
 *    itself is unreferenced and dropped by the linker);
 *  - romstage (assembly_entry.S) and postcar (arch/486/exit_car.S) call
 *    gdt_init() so that GDTR points to their own copy of the table.
 * Selectors: 0x08 flat 32-bit code, 0x10 flat 32-bit data
 * (arch/rom_segs.h). program.ld and bootblock.ld put .init first.
 */

.code32

.section .init, "ax", @progbits

.section .init._gdt_, "ax", @progbits

	/* void gdt_init(void): GDTR := gdtptr (6 bytes: 16-bit limit,
	 * 32-bit linear base). */
	.globl gdt_init
gdt_init:
/*
nop
nop
nop
nop
nop
nop
nop
	rdtsc
	rdtsc
	rdtsc
	rdtsc
*/
	lgdt	%cs:gdtptr
	ret

.previous
	.align	4
.globl gdtptr
gdtptr:
	.word	gdt_end - gdt -1 /* compute the table limit */
	.long	gdt		 /* we know the offset */

#if ENV_X86_64
.code64
.section .init._gdt64_, "ax", @progbits
	.globl gdt_init64
gdt_init64:
	movabs	$gdtptr64, %rax
	lgdt	(%rax)
	ret

.previous
	.align	4
.globl gdtptr64
gdtptr64:
	.word	gdt_end - gdt -1 /* compute the table limit */
	.quad	gdt		 /* we know the offset */
#endif

	/* Each descriptor is 8 bytes: limit[15:0], base[15:0], base[23:16],
	 * access byte, flags (G, D/B, L, AVL) in the high nibble with
	 * limit[19:16] in the low nibble, base[31:24].
	 * Access 0x9b = present, ring 0, code, execute/read, accessed;
	 * 0x93 = present, ring 0, data, read/write, accessed.
	 * Flags 0xc = G (limit in 4 KiB units) + D (32-bit): base 0 with limit
	 * 0xfffff * 4 KiB covers the whole 4 GiB, so offset = physical
	 * address ("flat"). */
	.align	4
gdt:
	/* selgdt 0, unused */
	/* (selector 0 is the null selector; the CPU never reads entry 0) */
	.word	0x0000, 0x0000		/* dummy */
	.byte	0x00, 0x00, 0x00, 0x00

	/* selgdt 0x08, flat code segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x9b, 0xcf, 0x00 /* G=1 and 0x0f, So we get 4Gbytes
					  for limit */

	/* selgdt 0x10, flat data segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x93, 0xcf, 0x00

	//TODO only 64bit??

	/* selgdt 0x18, flat code segment (64-bit) */
	/* (flags 0xa = G + L: x86-64 only, never loaded on a 486) */
	.word   0xffff, 0x0000
	.byte   0x00, 0x9b, 0xaf, 0x00

gdt_end:
