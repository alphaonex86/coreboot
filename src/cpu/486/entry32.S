/* SPDX-License-Identifier: GPL-2.0-or-later */

/* For starting coreboot in protected mode */

/*
 * This is the modern bootblock. It prepares the system for C environment runtime
 * setup. The actual setup is done by hardware-specific code.
 *
 * It provides a bootflow similar to other architectures, and thus is considered
 * to be the modern approach.
 *
 */

#include <arch/rom_segs.h>
#include <cpu/486/cr.h>
#include <cpu/486/post_code.h>

.section .init, "ax", @progbits

	.code32
/*
 *	When we come here we are in protected mode.
 *	NOTE aligned to 4 so that we are sure that the prefetch
 *	cache will be reloaded.
 */
	.align	4

/*
 * Entered from entry16.S by "ljmpl $ROM_CODE_SEG": 32-bit protected mode,
 * CS = 0x08 (flat 4 GiB code from gdt_init.S), paging off, L1 cache off,
 * interrupts off, no stack yet, EAX = BIST result. Still running in place
 * from flash. Exit: jump to bootblock_pre_c_entry (cache_as_ram.S).
 */
.globl bootblock_protected_mode_entry
bootblock_protected_mode_entry:

	/* Save the BIST value */
	movl	%eax, %ebp

	post_code(POST_ENTER_PROTECTED_MODE)

	/* DS/ES/SS := 0x10, the flat 4 GiB data segment (base 0): from now
	 * on an address is the physical address. FS/GS get the null
	 * selector, which is legal as long as they are not used. */
	movw	$ROM_DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	xor	%ax, %ax /* zero out the gs and fs segment index */
	movw	%ax, %fs
	movw	%ax, %gs /* Will be used for cpu_info */

	/* Restore the BIST value to %eax */
	movl	%ebp, %eax

#if CONFIG(BOOTBLOCK_DEBUG_SPINLOOP)

	/* Wait for a JTAG debugger to break in and set EBX non-zero */
	xor	%ebx, %ebx

debug_spinloop:
	cmp	$0, %ebx
	jz	debug_spinloop
#endif


/* Disabled: early TSC timestamp kept in MMX registers. A 486 has neither
 * MMX nor a time stamp counter. */
#if 0
#if !CONFIG(USE_MARCH_586 || USE_MARCH_486)
	/* MMX registers required here */

	/* BIST result in eax */
	movd	%eax, %mm0

__timestamp:

	/* Get an early timestamp */
	rdtsc
	movd	%eax, %mm1
	movd	%edx, %mm2
#endif
#endif

	/* We're done. Now it's up to platform-specific code */
	jmp	bootblock_pre_c_entry
