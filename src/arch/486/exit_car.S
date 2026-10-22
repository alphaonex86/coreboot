/* SPDX-License-Identifier: GPL-2.0-only */

//TODO remove 686 and x86-64 code

/*
 * Postcar entry point: leaves cache-as-RAM (cache_as_ram.S) and calls
 * main() in arch/486/postcar.c, which loads and runs ramstage.
 *
 * Romstage has initialized the DRAM, copied postcar into it (cbmem),
 * relocated it (postcar is an rmodule) and calls _start(cbmem_top) as a C
 * function (prog_run in arch/486/postcar_loader.c). On entry: 32-bit flat
 * protected mode, CR0.CD = NW = 1 (CAR still frozen in L1), ESP still on
 * the romstage stack in CAR, code executing from DRAM (uncached).
 *
 * This version tears CAR down inline (the call to chipset_teardown_car is
 * commented out). It also contains debug output that is always built:
 * raw binary bytes on COM1 (marked "pc2005" by the author).
 */


#include <cpu/486/cr.h>
#include <cpu/486/cache.h>
#include <cpu/486/post_code.h>


#define UMC_PCI_BDFR(BUS, DEV, FN, REG) ( \
	(1 << 31) | \
	(((BUS) & 0xff) << 16) | \
	(((DEV) & 0x1f) << 11) | \
	(((FN)  & 0x07) << 8) | \
	(((REG)  & 0xff) << 0) \
)


/* pci_write/pci_read: PCI configuration byte access, see cpu/486/cache_as_ram.S. */


#define pci_write(BDFR, REG_IMM)	\
	movw	$0xcf8, %dx;	\
	movl	$ BDFR, %eax;	\
	outl	%eax, %dx;	\
	movw	$0xcfc, %dx;	\
	and	$0x3, %al;	\
	add	%al, %dl;	\
	movb	REG_IMM, %al;	\
	outb	%al, %dx

//value returned in AL
#define pci_read(BDFR)		\
	movw	$0xcf8, %dx;	\
	movl	$ BDFR, %eax;	\
	outl	%eax, %dx;	\
	movw	$0xcfc, %dx;	\
	and	$0x3, %al;	\
	add	%al, %dl;	\
	inb	%dx, %al




/* Place the stack in the bss section. It's not necessary to define it in
 * the linker script. */
	.section .bss, "aw", @nobits
.global _stack
.global _estack
.global _stack_size

/* Postcar stack (CONFIG_STACK_SIZE, 8 KiB), in DRAM. */
_stack:
.space CONFIG_STACK_SIZE
_estack:
.set _stack_size, _estack - _stack

.text
.global _start
_start:
	/* Assume stack alignment doesn't matter here as chipset_teardown_car
	   is expected to be implemented in assembly. */

	post_code(POST_486_POSTCAR)

	/* Migrate GDT to this text segment */
	/* (GDTR := postcar's own copy of the ROM GDT, gdt_init.S) */
#if ENV_X86_64
	call	gdt_init64
#else
	call	gdt_init
#endif


#if ENV_X86_64
	mov	%rdi, %rax
	movabs %rax, _cbmem_top_ptr
#else
	/* The return argument is at 0(%esp), the calling argument at 4(%esp) */
	/* (save cbmem_top before the CAR stack is dropped; the write misses
	 * the cache and goes straight to DRAM) */
	movl	4(%esp), %eax
	movl	%eax, _cbmem_top_ptr
#endif



/* Disabled: clflush does not exist on a 486 (and is not needed: with
 * CD = NW = 1 the write above already went to DRAM). */
#if 0	//no cflush on 486 // TODO macro?
	/* Make sure _cbmem_top_ptr hits dram before invd */
	movl	$1, %eax
	cpuid
	btl	$CPUID_FEATURE_CLFLUSH_BIT, %edx
	jnc	skip_clflush
#if ENV_X86_64
	movabs	$_cbmem_top_ptr, %rax
	clflush	(%rax)
#else
	clflush	_cbmem_top_ptr
#endif
#endif

	# //page 135 137 83


	# movl	$0x3, %eax
	# movl	%eax, %tr5




skip_clflush:
	/* chipset_teardown_car() is expected to disable cache-as-ram. */
	# call	chipset_teardown_car

	# wbinvd	//pc2005

	# invd



	//fill L2 cache, twice as size?
	/* Zero 1 MiB of DRAM at 0x200000-0x2FFFFF (rep stosl: EAX to ECX
	 * dwords at ES:EDI). Still in CAR mode, so the writes miss L1 and go
	 * to DRAM (and through the board L2 cache, the apparent purpose).
	 * Nothing is loaded there yet: postcar and its stack are in cbmem
	 * near the top of RAM, ramstage and the payload come later. */
	movl	$0x200000, %edi
	movl	$0x300000, %ecx
	sub	%edi, %ecx
	shr	$2, %ecx
	cld

	# rep	lodsl	/* DF=0, up */

	xor	%eax, %eax
	rep	stosl	/* DF=0, up */

	/* Leave CAR: drop every L1 line without write-back. The old CAR
	 * content (romstage stack, pre-RAM console) is gone; nothing below
	 * uses the stack until ESP is reloaded. */
	invd

	/* Enable caching if not already enabled. */
	/* (CD = NW = 0: normal L1 operation from here on; the invd after it
	 * makes sure L1 starts empty) */
#if ENV_X86_64
	mov	%cr0, %rax
	and     $(~(CR0_CD | CR0_NW)), %eax
	mov	%rax, %cr0
#else
	mov	%cr0, %eax
	and	$(~(CR0_CD | CR0_NW)), %eax
	mov	%eax, %cr0
#endif
	/* Ensure cache is clean. */
	# invd

	# //hack puvodne invd pc2005
	# wbinvd
	invd
	# invd

	/* Read 2 MiB (ECX = 0x80000 dwords) with rep lodsl from DS:ESI, to
	 * fill the caches from 0x200000-0x3FFFFF. */
	post_code(POST_486_CAR_OFF)
	movl	$0x200000, %esi
	movl	$0x400000, %ecx
	sub	%esi, %ecx
	shr	$2, %ecx
	cld

	rep	lodsl	/* DF=0, up */

	# xor	%eax, %eax
	# rep	stosl	/* DF=0, up */


	# //dies here, WHY???

#if 0
	pci_read(UMC_PCI_BDFR(0, 0x10, 0, 0x5a))
	movb	%al, %cl
	andb	$(~0x40), %cl
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x5a), %cl)
#endif

	//TODO switch to writeback? (any time probably)
	/* Read-modify-write of UM8881 register 50h (HB50_L2_CFG_0): set bit
	 * 6, the L2 cache WT/WB mode bit (per ../um8881f-documentation-*.pdf,
	 * community notes, not an official datasheet). */
#if CONFIG(NORTHBRIDGE_UMC_UM8881)
	pci_read(UMC_PCI_BDFR(0, 0x10, 0, 0x50))
	movb	%al, %cl
	orb	$(0x40), %cl
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x50), %cl)
#endif



	//NOTICE all levels of memory should be coherent by now

	/* Switch to postcar's own stack in DRAM. */
	movl	$_estack, %esp
	/* Align stack to 16 bytes at call instruction. */
	andl	$0xfffffff0, %esp

	/* Call this in assembly as some platforms like to mess with the bootflow and
	   call into main directly from chipset_teardown_car. */
	/* call	postcar_mtrr_setup */

	/* Call into main for postcar. */
	/* (arch/486/postcar.c: console, cbmem, then run_ramstage()) */
	call	main
	/* Should never return. */
1:
	jmp	1b
