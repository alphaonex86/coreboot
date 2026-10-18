/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * This software and ancillary information (herein called SOFTWARE)
 * called LinuxBIOS is made available under the terms described here.
 *
 * The SOFTWARE has been approved for release with associated
 * LA-CC Number 00-34. Unless otherwise indicated, this SOFTWARE has
 * been authored by an employee or employees of the University of
 * California, operator of the Los Alamos National Laboratory under
 * Contract No. W-7405-ENG-36 with the U.S. Department of Energy.
 *
 * The U.S. Government has rights to use, reproduce, and distribute this
 * SOFTWARE. The public may copy, distribute, prepare derivative works
 * and publicly display this SOFTWARE without charge, provided that this
 * Notice and any statement of authorship are reproduced on all copies.
 *
 * Neither the Government nor the University makes any warranty, express
 * or implied, or assumes any liability or responsibility for the use of
 * this SOFTWARE.  If SOFTWARE is modified to produce derivative works,
 * such modified SOFTWARE should be clearly marked, so as not to confuse
 * it with the version available from LANL.
 *
 */


/* Start code to put an i386 or later processor into 32-bit protected mode.
 */

/*
 * Boot flow: reset16.S (reset vector) jumps here, in real mode, running in
 * place from flash at 0xFFFFxxxx. This code:
 *   1. saves the BIST result, sets up COM1 for the POST characters,
 *   2. flushes the TLB and the L1 cache,
 *   3. on a CPU-only reset with the chipset still configured, hands the
 *      reset to SeaBIOS in shadow RAM (far jump to F000:FFF0),
 *   4. otherwise loads an empty IDT and the ROM GDT (gdt_init.S), sets
 *      CR0.PE and far-jumps to bootblock_protected_mode_entry (entry32.S),
 *      which runs in 32-bit protected mode.
 * Register use: EBP keeps the BIST result (EAX at reset) until entry32.S;
 * DX/AL are scratch for port I/O.
 */

#include <arch/rom_segs.h>
#include <cpu/486/post_code.h>





.section .init._start, "ax", @progbits

/* Symbol _start16bit must reachable from the reset vector, and be aligned to
 * 4kB to start AP CPUs with Startup IPI message without RAM.
 */
.code16
.globl _start16bit
.type _start16bit, @function

_start16bit:
	/* Real mode, 16-bit code. Interrupts stay off until the OS: nothing
	 * could handle them yet. */
	cli
	/* Save the BIST result */
	/* (EAX = built-in self-test result after reset, 0 = pass; EBP is not
	 * used for anything else until entry32.S.) */
	movl	%eax, %ebp

	/* Keep the CPU signature that DX holds after reset (bits 11-4: family
	 * and model) in CMOS 7Fh, for the bus clock and the name of CPUs
	 * without CPUID (newbios_cmos.h NB_CMOS_CPUSIG). Port 70h bit 7 (NMI
	 * mask) is kept as read. Skipped if DX is 0. */
	testw	%dx, %dx
	jz	1f
	inb	$0x70, %al
	andb	$0x80, %al
	orb	$0x7f, %al
	outb	%al, $0x70
	movw	%dx, %ax
	shrw	$4, %ax
	outb	%al, $0x71
1:

	/* With CONFIG_POST_IO each post_code(c) below writes c to port 80h
	 * for a POST card (include/cpu/486/post_code.h, mb-8433UUD-A-BIOS/POSTCODES.md).
	 * post_init is empty (it used to set up COM1 for them). */
	post_init
	post_code(POST_RESET_VECTOR_CORRECT)

	/* IMMEDIATELY invalidate the translation lookaside buffer (TLB) before
	 * executing any further code. Even though paging is disabled we
	 * could still get false address translations due to the TLB if we
	 * didn't invalidate it. Thanks to kmliu@sis.com.tw for this TLB fix.
	 */

	xorl	%eax, %eax
	movl	%eax, %cr3    /* Invalidate TLB*/

	/* Invalidating the cache here seems to be a bad idea on
	 * modern processors.  Don't.
	 * If we are hyperthreaded or we have multiple cores it is bad,
	 * for SMP startup.  On Opterons it causes a 5 second delay.
	 * Invalidating the cache was pure paranoia in any event.
	 * If your CPU needs it you can write a CPU dependent version of
	 * entry16.inc.
	 */

	 //486 is not a modern processor :-D
	 /* invd drops every L1 line without writing it back. */
	 invd

	/* check if chipset is configured
	 * = segment E/F is shadowed in RAM -> contains seabios -> jump there
	 */
	/* A CPU-only reset (port 92h fast reset, keyboard controller reset,
	 * triple fault) does not reset the chipset. If the E and F segments
	 * are still read from shadow RAM, SeaBIOS is loaded there and can
	 * handle the reset itself (CMOS shutdown code, e.g. the 286-style
	 * return from protected mode), so coreboot is skipped. */
#if CONFIG(NORTHBRIDGE_ALI_M1489)
	//TODO
#elif CONFIG(NORTHBRIDGE_UMC_UM8881)
	//if 0x54 and 0x55 has shadow set -> guess the chipset is configured
	//NOTICE cannot check ram size register, minimum RAM is defined at power on

	/* PCI configuration read, mechanism #1: write the address
	 * (bit 31 enable | bus 0 | device 0x10 | function 0 | register 0x54,
	 * the UM8881 host bridge) to port 0xCF8, then read the byte at
	 * 0xCFC + (register & 3). */
	movw	$0xcf8, %dx
	movl	$0x80008054, %eax	//BDFR: 0:0x10:0
	outl	%eax, %dx
	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	inb	%dx, %al

	/* Register 54h bit 0: E0000-EFFFF read from shadow RAM. */
	test	$0x01, %al
	jz	no_soft_reset	/* segment E not shadowed */

	/* Same read for register 55h; bit 7 = global shadow read enable,
	 * which also selects RAM for F0000-FFFFF. */
	movw	$0xcf8, %dx
	movl	$0x80008055, %eax	//BDFR: 0:0x10:0
	outl	%eax, %dx
	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	inb	%dx, %al

	test	$0x80, %al
	jz	no_soft_reset	/* segment F not shadowed, global shadow disabled */


#if 1
	//read register 0xf of RTC
	/* CMOS byte 0Fh = shutdown status, set by software before a CPU
	 * reset. Port 70h selects the CMOS register; its bit 7 masks NMI
	 * and is kept as read. 0 = normal reset: do the full coreboot boot. */
	inb	$0x70, %al
	and	$0x80, %al
	or	$0xf, %al
	outb	%al, $0x70
	inb	$0x71, %al
	cmp	$0, %al
	jz	no_soft_reset
#endif



	//also if no cmos/resume != 0 -> hard reset

#if 0
	jmp	no_soft_reset	//TEMP
#endif

	//chipset is configured -> soft reset (CPU only reset)
	//jump to shadow (seabios or other payload??)

	post_code(POST_486_SOFT_RESET)



	/* Restore BIST to %eax */
	movl	%ebp, %eax

	//TODO restore EDX?

	/* Real-mode far jump to F000:FFF0 = physical 0xFFFF0, which now
	 * reads shadow RAM: the SeaBIOS reset_vector (romlayout.S) ->
	 * entry_post -> entry_resume, as if the CPU had reset there. */
	ljmpl	$0xf000, $0xfff0

no_soft_reset:
#endif

	/* Note: gas handles memory addresses in 16 bit code very poorly.
	 * In particular it doesn't appear to have a directive allowing you
	 * associate a section or even an absolute offset with a segment register.
	 *
	 * This means that anything except cs:ip relative offsets are
	 * a real pain in 16 bit mode.  And explains why it is almost
	 * impossible to get gas to do lgdt correctly.
	 *
	 * One way to work around this is to have the linker do the
	 * math instead of the assembler.  This solves the very
	 * practical problem of being able to write code that can
	 * be relocated.
	 *
	 * An lgdt call before we have memory enabled cannot be
	 * position independent, as we cannot execute a call
	 * instruction to get our current instruction pointer.
	 * So while this code is relocatable it isn't arbitrarily
	 * relocatable.
	 *
	 * The criteria for relocation have been relaxed to their
	 * utmost, so that we can use the same code for both
	 * our initial entry point and startup of the second CPU.
	 * The code assumes when executing at _start16bit that:
	 * (((cs & 0xfff) == 0) and (ip == _start16bit & 0xffff))
	 * or
	 * ((cs == anything) and (ip == 0)).
	 *
	 * The restrictions in reset16.inc mean that _start16bit initially
	 * must be loaded at or above 0xffff0000 or below 0x100000.
	 *
	 * The linker scripts computes gdtptr16_offset by simply returning
	 * the low 16 bits.  This means that the initial segment used
	 * when start is called must be 64K aligned.  This should not
	 * restrict the address as the ip address can be anything.
	 *
	 * Also load an IDT with NULL limit to prevent the 16bit IDT being used
	 * in protected mode before c_start.S sets up a 32bit IDT when entering
	 * RAM stage. In practise: CPU will shutdown on any exception.
	 * See IA32 manual Vol 3A 19.26 Interrupts.
	 */

	/* BX = offset of the descriptor-table image from the CS base.
	 * bootblock.ld defines nullidt_offset/gdtptr_offset as the low 16
	 * bits of the linked addresses (0xFFFFFExx). At reset CS = 0xF000 and
	 * (CS << 4) truncated to 16 bits is 0, so BX is the offset itself and
	 * %cs:(%bx) reads 0xFFFF0000 + offset, i.e. the flash. */
	movw	%cs, %ax
	shlw	$4, %ax
	movw	$nullidt_offset, %bx
	subw	%ax, %bx
	/* IDTR := limit 0: no interrupt or exception can be delivered in
	 * protected mode; one would triple-fault (CPU shutdown cycle).
	 * It stays so in romstage and postcar; ramstage loads
	 * a real IDT (idt.S, exception.c). */
	lidt	%cs:(%bx)
	movw	$gdtptr_offset, %bx
	subw	%ax, %bx
	/* GDTR := gdtptr (gdt_init.S). The "l" suffix (32-bit operand size)
	 * is required: the 16-bit form loads only 24 bits of the base and
	 * would miss 0xFFFFFExx. */
	lgdtl	%cs:(%bx)

	post_code(POST_486_GDT)

	/* Enter protected mode: PE = 1. CD = NW = 1 keep the L1 cache
	 * disabled (cache_as_ram.S enables it). Paging, alignment check,
	 * write protect, task switched and FPU emulation are cleared; NE = 0
	 * reports FPU errors through IRQ13 as on a PC. */
	movl	%cr0, %eax
	andl	$0x7FFAFFD1, %eax /* PG,AM,WP,NE,TS,EM,MP = 0 */
	orl	$0x60000001, %eax /* CD, NW, PE = 1 */
	movl	%eax, %cr0

	/* Now in 16-bit protected mode: CS keeps its hidden real-mode
	 * descriptor (base 0xFFFF0000) until the far jump below reloads it. */

	//pc2005 test CS
	post_code(POST_486_PROTECTED_MODE)

	/* Restore BIST to %eax */
	movl	%ebp, %eax

/*
	nop
	nop
	nop
	nop
	rdtsc*/


	/* Now that we are in protected mode jump to a 32 bit code segment. */
	/* Far jump with a 32-bit offset: CS := selector 0x08 (flat 4 GiB
	 * 32-bit code), EIP := bootblock_protected_mode_entry (entry32.S).
	 * This also flushes the prefetch queue. */
	ljmpl	$ROM_CODE_SEG, $bootblock_protected_mode_entry

	/**
	 * The gdt is defined in gdt_init.S, it has a 4 Gb code segment
	 * at 0x08, and a 4 GB data segment at 0x10;
	 */
	/* __gdtptr is an unused leftover: nothing references it (the lgdtl
	 * above uses gdtptr_offset). */
__gdtptr:
	.long	gdtptr

/* IDTR image for the lidt above: 16-bit limit 0, 32-bit base 0, then 2
 * bytes of padding. */
.align	4
.globl nullidt
nullidt:
	.word	0	/* limit */
	.long	0
	.word	0
