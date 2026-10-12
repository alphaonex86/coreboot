/* SPDX-License-Identifier: GPL-2.0-only */

#include <cpu/486/cache.h>
#include <cpu/486/post_code.h>
#include <cpu/intel/post_codes.h>

/* pc2005 TODO */
#define send_uart		\
	movw	$0x3f8, %dx;	\
	outb	%al, %dx;	\
1:	movw	$0x3fd, %dx;	\
	inb	%dx, %al;	\
	testb	$0x20, %al;	\
	jz	1b


/*
 * chipset_teardown_car(): leave cache-as-RAM (see cache_as_ram.S) by
 * disabling the cache, then return to the caller.
 *
 * NOT IN THE ROM: the only caller, _start in arch/486/exit_car.S, has the
 * call commented out, and the linker drops this unreferenced .text
 * (postcar.map lists it under "Discarded input sections"). Postcar
 * leaves CAR inline in arch/486/exit_car.S instead.
 */
.global chipset_teardown_car

.code32
chipset_teardown_car:
#if 0

	# [esp] = 00ff213f
	mov	(%esp), %eax
	send_uart
	shr	$0x8, %eax
	send_uart
	shr	$0x8, %eax
	send_uart
	shr	$0x8, %eax
	send_uart

	mov	$0xd, %al	//pc2005
	send_uart
#endif


	/* The stack lives in CAR and is about to disappear: pop the return
	 * address into ESP itself (no memory needed) and jump to it below. */
	pop %esp

	post_code(POST_POSTCAR_DISABLE_CACHE)	/*'0'*/

	/* Disable cache. */
	movl	%cr0, %eax
	orl	$CR0_CacheDisable, %eax
	movl	%eax, %cr0

	post_code(POST_POSTCAR_TEARDOWN_DONE)	/*'2'*/

	////////////

	# movl	$0x3, %eax
	# movl	%eax, %tr5

	# invd

#if 0
	# 0x00ff213f
	mov	%esp, %eax
	send_uart
	shr	$0x8, %eax
	send_uart
	shr	$0x8, %eax
	send_uart
	shr	$0x8, %eax
	send_uart

	mov	$0xd, %al	//pc2005
	send_uart

	////////////

/*	# 0x00ff2380
	call	test
test:
	pop	%eax
	send_uart
	shr $0x8, %eax
	send_uart
	shr $0x8, %eax
	send_uart
	shr $0x8, %eax
	send_uart

	mov	$0xd, %al	//pc2005
	send_uart*/


	post_code('}')
#endif

#if 0
	# //quick search debug block
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rdtsc
#endif


	/* Return to caller. */
	/* ESP still holds the return address, not a stack: the caller must
	 * load a new ESP before it uses the stack. */
	jmp	*%esp
