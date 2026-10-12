/* SPDX-License-Identifier: GPL-2.0-only */

#include <cpu/486/post_code.h>
#include <arch/ram_segs.h>

/*
 * Ramstage entry point and the ramstage GDT.
 * Postcar loads ramstage from CBFS into RAM (cbmem), relocates it (it is an
 * rmodule) and calls _start(cbmem_top) as a C function: 32-bit flat
 * protected mode, caches on, interrupts off, still on the postcar stack.
 * _start switches to its own GDT and stack and calls main()
 * (lib/hardwaremain.c), which never returns.
 */

/* Place the stack in the bss section. It's not necessary to define it in
 * the linker script. */
	.section .bss, "aw", @nobits
.global _stack
.global _estack
.global _stack_size

/* CONFIG_STACK_SIZE (8 KiB) of ramstage stack, in RAM. */
.align 16
_stack:
.space CONFIG_STACK_SIZE
_estack:
.set _stack_size, _estack - _stack

	.section ".text._start", "ax", @progbits
#if ENV_X86_64
	.code64
#else
	.code32
#endif
	.globl _start
_start:
	cli
	/* GDTR := the GDT below (in .data, so in RAM). The far jump reloads
	 * CS with RAM_CODE_SEG = 0x10, the flat code segment of this table:
	 * the selector numbers differ from the ROM GDT (code 0x08, data 0x10).
	 * Then DS/ES/SS := RAM_DATA_SEG = 0x18, FS/GS := null selector. */
#if ENV_X86_64
	movabs	$gdtaddr, %rax
	lgdt	(%rax)
#else
	lgdt	%cs:gdtaddr
	ljmp	$RAM_CODE_SEG, $1f
#endif
1:	movl	$RAM_DATA_SEG, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %ss
	xor	%eax, %eax /* zero out the gs and fs segment index */
	movl	%eax, %fs
	movl	%eax, %gs /* Will be used for cpu_info */
#if ENV_X86_64
	mov	$RAM_CODE_SEG64, %ecx
	call	SetCodeSelector
#endif

	post_code(POST_ENTRY_C_START)		/* post 13 */

	cld

#if ENV_X86_64
	mov	%rdi, %rax
	movabs	%rax, _cbmem_top_ptr
	movabs	$_stack, %rdi
#else
	/* The return argument is at 0(%esp), the calling argument at 4(%esp) */
	/* (_cbmem_top_ptr: read back by cbmem_top() in lib/imd_cbmem.c) */
	movl	4(%esp), %eax
	movl	%eax, _cbmem_top_ptr
	leal	_stack, %edi
#endif

	/** poison the stack. Code should not count on the
	 * stack being full of zeros. This stack poisoning
	 * recently uncovered a bug in the broadcast SIPI
	 * code.
	 */
	/* rep stosl: store EAX to ECX dwords at ES:EDI (EDI = _stack). */
	movl	$_estack, %ecx
	subl	%edi, %ecx
	shrl	$2, %ecx   /* it is 32 bit aligned, right? */
	movl	$0xDEADBEEF, %eax
	rep
	stosl

	/* Set new stack with enforced alignment. */
	movl	$_estack, %esp
	andl	$(0xfffffff0), %esp

	/*
	 *	Now we are finished. Memory is up, data is copied and
	 *	bss is cleared.   Now we call the main routine and
	 *	let it do the rest.
	 */
	post_code(POST_PRE_HARDWAREMAIN)	/* post 6e */

	andl	$0xFFFFFFF0, %esp

#if CONFIG(ASAN_IN_RAMSTAGE)
	call asan_init
#endif

#if CONFIG(GDB_WAIT)
	call gdb_hw_init
	call gdb_stub_breakpoint
#endif
	/* lib/hardwaremain.c: loads the IDT (exception_init), enumerates
	 * and configures the devices, writes the tables, loads and starts
	 * the payload (SeaBIOS). */
	call	main
	/* NOTREACHED */
.Lhlt:
	post_code(POST_DEAD_CODE)	/* post ee */
	hlt
	jmp	.Lhlt

#if CONFIG(GDB_WAIT)

	.globl gdb_stub_breakpoint
gdb_stub_breakpoint:
#if ENV_X86_64
	pop	%rax	/* Return address */
	pushfl
	push	%cs
	push	%rax	/* Return address */
	push	$0	/* No error code */
	push	$32	/* vector 32 is user defined */
#else
	popl	%eax	/* Return address */
	pushfl
	pushl	%cs
	pushl	%eax	/* Return address */
	pushl	$0	/* No error code */
	pushl	$32	/* vector 32 is user defined */
#endif
	jmp	int_hand
#endif

	.globl gdt, gdt_end
	.global per_cpu_segment_descriptors, per_cpu_segment_selector

gdtaddr:
	.word	gdt_end - gdt - 1
#if ENV_X86_64
	.quad	gdt
#else
	.long	gdt		/* we know the offset */
#endif

	.data

	/* This is the gdt for GCC part of coreboot.
	 * It is different from the gdt in ASM part of coreboot
	 * which is defined in gdt_init.S
	 *
	 * When the machine is initially started, we use a very simple
	 * gdt from ROM (that in gdt_init.S) which only contains those
	 * entries we need for protected mode.
	 *
	 * When we're executing code from RAM, we want to do more complex
	 * stuff, like initializing PCI option ROMs in real mode, or doing
	 * a resume from a suspend to RAM.
	 */
	/* Descriptor format: see gdt_init.S. In this build only 0x10, 0x18
	 * and the per-CPU entry are used: option ROMs run under SeaBIOS, not
	 * coreboot, and ACPI S3 resume is not built. */
gdt:
	/* selgdt 0, unused */
	.word	0x0000, 0x0000		/* dummy */
	.byte	0x00, 0x00, 0x00, 0x00

	/* selgdt 8, unused */
	.word	0x0000, 0x0000		/* dummy */
	.byte	0x00, 0x00, 0x00, 0x00

	/* selgdt 0x10, flat code segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x9b, 0xcf, 0x00 /* G=1 and 0x0f, So we get 4Gbytes for
					* limit
					*/

	/* selgdt 0x18, flat data segment */
	.word	0xffff, 0x0000
#if ENV_X86_64
	.byte	0x00, 0x92, 0xcf, 0x00
#else
	.byte	0x00, 0x93, 0xcf, 0x00
#endif

	/* selgdt 0x20, unused */
	.word	0x0000, 0x0000		/* dummy */
	.byte	0x00, 0x00, 0x00, 0x00

	/* The next two entries are used for executing VGA option ROMs */

	/* (flags 0: byte granularity, 16-bit; limit 0xffff) */
	/* selgdt 0x28 16 bit 64k code at 0x00000000 */
	.word	0xffff, 0x0000
	.byte	0, 0x9a, 0, 0

	/* selgdt 0x30 16 bit 64k data at 0x00000000 */
	.word	0xffff, 0x0000
	.byte	0, 0x92, 0, 0

	/* The next two entries are used for ACPI S3 RESUME */
	/* (Unused here. arch/ram_segs.h names 0x38 code and 0x40 data, the
	 * reverse of the comments below, and the limit[15:0] of 0x38 is 0,
	 * so its limit is 0xF0000FFF rather than 4 GiB; both as upstream.) */

	/* selgdt 0x38, flat data segment 16 bit */
	.word	0x0000, 0x0000		/* dummy */
	.byte	0x00, 0x93, 0x8f, 0x00 /* G=1 and 0x0f, So we get 4Gbytes for
					* limit
					*/

	/* selgdt 0x40, flat code segment 16 bit */
	.word	0xffff, 0x0000
	.byte	0x00, 0x9b, 0x8f, 0x00 /* G=1 and 0x0f, So we get 4Gbytes for
					* limit
					*/

#if ENV_X86_64
	/* selgdt 0x48, flat x64 code segment */
	.word	0xffff, 0x0000
	.byte	0x00, 0x9b, 0xaf, 0x00
#endif
	/* One data descriptor per CPU (CONFIG_MAX_CPUS = 1 here).
	 * set_cpu_info() in arch/486/cpu.c patches its base to the CPU data
	 * and loads it into GS; cpu_info() then reads the pointer at gs:0. */
per_cpu_segment_descriptors:
	.rept CONFIG_MAX_CPUS
	/* flat data segment */
	.word	0xffff, 0x0000
#if ENV_X86_64
	.byte	0x00, 0x92, 0xcf, 0x00
#else
	.byte	0x00, 0x93, 0xcf, 0x00
#endif
	.endr
gdt_end:

/* Segment selector pointing to the first per_cpu_segment_descriptor. */
per_cpu_segment_selector:
	.long	per_cpu_segment_descriptors - gdt

	.section ".text._start", "ax", @progbits
#if ENV_X86_64
SetCodeSelector:
	# save rsp because iret will align it to a 16 byte boundary
	mov	%rsp, %rdx

	# use iret to jump to a 64-bit offset in a new code segment
	# iret will pop cs:rip, flags, then ss:rsp
	mov	%ss, %ax	# need to push ss..
	push	%rax		# push ss instruction not valid in x64 mode,
				# so use ax
	push	%rsp
	pushfq
	push	%rcx		# cx is code segment selector from caller
	movabs	$setCodeSelectorLongJump, %rax
	push	%rax

	# the iret will continue at next instruction, with the new cs value
	# loaded
	iretq

setCodeSelectorLongJump:
	# restore rsp, it might not have been 16-byte aligned on entry
	mov	%rdx, %rsp
	ret
#endif
