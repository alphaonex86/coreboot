/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Cache-as-RAM (CAR) for the 486, then the call into the bootblock C code.
 *
 * Until romstage has initialized the DRAM there is no RAM, but C code needs
 * a stack and some writable data. The 486 L1 cache provides it:
 *   1. make the CAR range (CONFIG_DCACHE_RAM_BASE, 8 KiB at 0x1A0000 in this
 *      build) look like cacheable DRAM to the chipset (fake DRAM banks), so
 *      the CPU may allocate cache lines for it (the chipset asserts KEN#);
 *   2. enable the cache (CR0.CD = NW = 0) and read the whole range once:
 *      every 16-byte line is filled into L1;
 *   3. set CR0.CD = 1 and NW = 1. In that mode the 486 makes no new line
 *      fills and no write-through: reads and writes that hit a cached line
 *      stay inside the cache, everything else goes to the bus uncached.
 *      The filled range is now 8 KiB of RAM inside the CPU, while code keeps
 *      being fetched uncached from flash;
 *   4. remove the fake banks, zero the range, point ESP into it and call
 *      bootblock_c_entry_bist() (src/cpu/486/bootblock.c), never to return.
 * The layout of the range (stack, pre-RAM console, timestamps, .bss) comes
 * from car.ld. Postcar (arch/486/exit_car.S) leaves this mode again.
 * Entered from entry32.S: 32-bit flat protected mode, EBP = BIST result.
 */

#include <cpu/486/post_code.h>
#include <cpu/486/cache.h>

/* PCI configuration address for mechanism #1 (port 0xCF8): enable bit 31,
 * bus, device, function and register number. */
#define UMC_PCI_BDFR(BUS, DEV, FN, REG) ( \
	(1 << 31) | \
	(((BUS) & 0xff) << 16) | \
	(((DEV) & 0x1f) << 11) | \
	(((FN)  & 0x07) << 8) | \
	(((REG)  & 0xff) << 0) \
)

/* Write one byte to PCI configuration space without a stack: address to
 * 0xCF8, then the byte to 0xCFC + (register & 3). Clobbers EAX, DX.
 * pci_read() is the same read; send_uart() writes AL to the UART and waits
 * until it is sent. */
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

#define UART_BASE	0x3f8

#define send_uart(base)		\
	movw	$(base+0), %dx;	\
	outb	%al, %dx;	\
1:	movw	$(base+5), %dx;	\
	inb	%dx, %al;	\
	testb	$0x20, %al;	\
	jz	1b



/* TODO use only vanilla

https://elixir.bootlin.com/coreboot/4.20.1/source/src/commonlib/include/commonlib/console/post_codes.h
*/
#include <cpu/intel/post_codes.h>

/* CBFS_FILE_*: unused leftovers (offsets of a CBFS file header). */
#define CBFS_FILE_MAGIC 0
#define CBFS_FILE_LEN (CBFS_FILE_MAGIC + 8)
#define CBFS_FILE_TYPE (CBFS_FILE_LEN + 4)
#define CBFS_FILE_CHECKSUM (CBFS_FILE_TYPE + 4)
#define CBFS_FILE_OFFSET (CBFS_FILE_CHECKSUM + 4)

.section .init, "ax", @progbits
.code32

.global bootblock_pre_c_entry
bootblock_pre_c_entry:

cache_as_ram:
	post_code(POST_BOOTBLOCK_CAR)


/* Disabled (#if 0): POKER, a tiny serial monitor for bring-up. A host sends
 * 6-byte commands on COM1 to read/write memory, I/O ports and PCI registers
 * and to switch the L1 cache before any C code runs. Not built. */
#if 0
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER

/*
NOTICE
command sequence (bytes)
	OP VALUE MSB3 mSB2 lSB1 LSB0
return
	1 byte, either read value, or dummy/ack 0x42
OP:
	q	quit
	r/w	read/write memory
	i/o	read/write port
	C/c	enable/disable L1 cache
	!	invalidate L1
	?	writeback invalidate L1
	p/P	read/write PCI
	l/L	loop (N*65536) write 0/0xffffffff to specified memory
*/


#if 1
	/* init uart */
	movw	$(UART_BASE+1), %dx
	movb	$0x0, %al
	outb	%al, %dx
	movw	$(UART_BASE+3), %dx
	movb	$0x80, %al
	outb	%al, %dx
	movw	$(UART_BASE+0), %dx
	movb	$1, %al
	outb	%al, %dx
	movw	$(UART_BASE+1), %dx
	movb	$0x0, %al
	outb	%al, %dx
	movw	$(UART_BASE+3), %dx
	inb	%dx, %al
	andb	$0x7f, %al
	outb	%al, %dx
	movw	$(UART_BASE+1), %dx
	movb	$0x00, %al
	outb	%al, %dx
	movw	$(UART_BASE+2), %dx
	movb	$0xc7, %al
	outb	%al, %dx
	movw	$(UART_BASE+3), %dx
	movb	$0x03, %al
	outb	%al, %dx
	movw	$(UART_BASE+4), %dx
	movb	$0x01, %al
	outb	%al, %dx
#endif

#if 0
	movw	$(UART_BASE+5), %dx
	inb	%dx, %al
	test	$0x10, %al	# break indicated
	jz	exit_poker
#elif 0
	movw	$(UART_BASE+6), %dx
	inb	%dx, %al
	test	$0x80, %al	# activated carrier detect
	# test	$0x20, %al	# activated DSR
	# test	$0x10, %al	# activated CTS
	jz	exit_poker
#endif

	mov	$'P', %al
	send_uart(UART_BASE)
	mov	$'O', %al
	send_uart(UART_BASE)
	mov	$'K', %al
	send_uart(UART_BASE)
	mov	$'E', %al
	send_uart(UART_BASE)
	mov	$'R', %al
	send_uart(UART_BASE)
	mov	$'>', %al
	send_uart(UART_BASE)

poker:
	xor	%edi, %edi	#command
	xor	%esi, %esi	#parameter
	#edi .. esi
	# op value .. a3 a2 a1 a0

	mov	$6, %ecx

	# wait until rx
wait_rx:
	mov	$(UART_BASE+5), %dx
	in	%dx, %al
	test	$1, %al
	jz	wait_rx

	# read byte
	mov	$(UART_BASE+0), %dx
	in	%dx, %al

	mov	%esi, %ebx
	mov 	%al, %bl
	mov	%ebx, %esi

	dec	%ecx
	jcxz	process

	#count src dst
	# src >> dst

	shld	$8, %esi, %edi
	shl	$8, %esi

	jmp	wait_rx

process:

	#BX command (BH op, BL value)
	#EBP parameter

	mov	%edi, %ebx

	# quit
	cmp	$'q', %bh
	je	exit_poker

	# read/write memory
	cmp	$'r', %bh
	je	cmd_read_addr
	cmp	$'w', %bh
	je	cmd_write_addr

	# read/write port
	cmp	$'i', %bh
	je	cmd_read_port
	cmp	$'o', %bh
	je	cmd_write_port

	# cache
	cmp	$'C', %bh
	je	cmd_l1_enable
	cmp	$'c', %bh
	je	cmd_l1_disable
	cmp	$'!', %bh
	je	cmd_l1_invalidate
	cmp	$'?', %bh
	je	cmd_l1_writebackinvalidate

	# pci
	cmp	$'p', %bh
	je	cmd_read_pci
	cmp	$'P', %bh
	je	cmd_write_pci

	# loop write memory
	cmp	$'l', %bh
	je	cmd_loop_write_0
	cmp	$'L', %bh
	je	cmd_loop_write_ffffffff

	jmp	poker

send_reply:
	# data in al, waits automatically
	send_uart(UART_BASE)
	jmp	poker

cmd_read_addr:
	mov	%ds:(%esi), %al
	jmp	send_reply

cmd_write_addr:
	mov	%bl, %ds:(%esi)
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_read_port:
	mov	%esi, %edx
	in	%dx, %al
	jmp	send_reply

cmd_write_port:
	mov	%bl, %al
	mov	%esi, %edx
	out	%al, %dx
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_l1_enable:
	movl	%cr0, %eax
	andl	$(~(CR0_CacheDisable | CR0_NoWriteThrough)), %eax
	movl	%eax, %cr0
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_l1_disable:
	movl	%cr0, %eax
	orl	$(CR0_CacheDisable | CR0_NoWriteThrough), %eax
	movl	%eax, %cr0
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_l1_invalidate:
	invd
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_l1_writebackinvalidate:
	wbinvd			#NOTICE will crash on WT-only 486
	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_read_pci:
	movw	$0xcf8, %dx
	movl	%esi, %eax
	outl	%eax, %dx
	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	inb	%dx, %al

	jmp	send_reply

cmd_write_pci:
	movw	$0xcf8, %dx
	movl	%esi, %eax
	outl	%eax, %dx
	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	mov	%bl, %al	#write the value
	outb	%al, %dx

	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_loop_write_0:
	xor	%ecx, %ecx
	mov	%bl, %cl
	shl	$16, %ecx
1:
	movl	$0, %ds:(%esi)
	loop	1b

	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

cmd_loop_write_ffffffff:
	xor	%ecx, %ecx
	mov	%bl, %cl
	shl	$16, %ecx
1:
	movl	$0xffffffff, %ds:(%esi)
	loop	1b

	mov	$0x42, %al	#dummy/ack reply
	jmp	send_reply

# # # # # # # # # # # # # # #

exit_poker:

#endif
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER
# POKER POKER POKER POKER POKER POKER POKER POKER POKER


# set up L1 cacheable range (usually it means to define fake RAM area)

/* The 486 allocates a cache line only when the chipset asserts KEN# for
 * the access, and a chipset does that only for its (cacheable) DRAM. The
 * blocks below make the CAR range part of the DRAM map. The DRAM content
 * itself does not matter: only the line fills are needed. ALi M1489 is
 * another board; this build uses the UMC UM8881 branch. */
#if CONFIG(NORTHBRIDGE_ALI_M1489)
	/* ALI specific, NB will not generate /KEN in noncacheable regions, probably everything outside RAM */

	/*unlock chipset*/
	movb	$3, %al
	outb	%al, $0x22
	movb	$0xc5, %al
	outb	%al, $0x23

	/*enable /KEN generation for L1*/
	movb	$0x16, %al
	outb	%al, $0x22
	movb	$(0x1 | 0x30), %al
	outb	%al, $0x23

	/* FAKE max RAM, so KEN gets generated ... TODO check */
	/* when 486 chipset resets the RAM is lost anyway? */
	movb	$0x10, %al
	outb	%al, $0x22
	movb	$0x44, %al
	outb	%al, $0x23

	movb	$0x11, %al
	outb	%al, $0x22
	movb	$0x44, %al
	outb	%al, $0x23

	/* TODO ?? enable 0xe0000 seems to be used before northbridge init */
	/* bootblock_soc_early_init which sets northbridge is called from e0000 region*/
	/* nevermind fffe0000 is not decoded in ALI and e0000 is not used in CB*/
	movb	$0x12, %al
	outb	%al, $0x22
	movb	$0x81, %al
	outb	%al, $0x23
#elif CONFIG(NORTHBRIDGE_UMC_UM8881)


#if 0	/* ========================= PCI DUMP ================ */
	movb	$0x0, %cl
pci_dump_loop:
	movw	$0xcf8, %dx
	movl	$ UMC_PCI_BDFR(0, 0x12, 1, 0), %eax
	movb	%cl, %al
	outl	%eax, %dx

	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	inb	%dx, %al

	send_uart(UART_BASE)

	inc	%cl

	//overflow
	cmp	$0x00, %cl
//	cmp	$0xb0, %cl
	jne	pci_dump_loop
#endif	/* ========================= PCI DUMP ================ */

	/*
	expects from poweron/reset:
		L2 disabled
		no forced L2 hit
		disabled ROM shadow, ROM cacheable
	*/

	/* define fake RAM 4x 32MB */
	/* CONFIG_DCACHE_RAM_BASE must be in range to L1 able to work as CAR */
	/* Host bridge (device 0x10) register 52h: bits 6-4 = upper region
	 * capacity, bits 2-0 = lower region capacity; 7 is the largest code
	 * (see ../um8881f-documentation-*.pdf). */
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x52), $0x77)

	/* 2 double sided banks in both */
	/* Register 53h bits 3-0: upper/lower region double sided, 2 banks. */
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x53), $0x0f)

	/* TODO EXPERIMENTAL set L1 bit ??? */
	# # # pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x51), $0x02)

#elif CONFIG(NORTHBRIDGE_OPTI_VIPERM)
	/* Nothing to set: the 82C557M powers up with a 2 MB DRAM configuration
	 * (index 00h), so 0x1A0000 is cacheable DRAM already (inferred). */
#endif

#if 0
	post_code(/*POST_SOC_ENABLE_CACHE*/ '5')
#endif

	/*
	https://en.wikipedia.org/wiki/Control_register#CR0
	intel 486 pdf page 53
	*/

	/* Enable cache (CR0.CD = 0, CR0.NW = 0) */
	/* The invd right before empties L1, so no stale line is enabled. */
	movl	%cr0, %eax
	andl	$(~(CR0_CacheDisable | CR0_NoWriteThrough)), %eax
	invd
	movl	%eax, %cr0

	/*
		p19 i486
		every mem write now goes outside CPU
		read will fill L1
	*/

	post_code(POST_486_CAR_FILL)

	/*
	address is defined in CONFIG_DCACHE_RAM_BASE
	must be in a defined RAM range to be captured in L1
	shouldn't be in first 1MB so it won't interfere with ROMs
	*/

	/* Fill cache lines */
	/* rep lodsl = load ECX dwords from DS:ESI into EAX, ESI += 4 each
	 * (DF = 0 after cld). The values are discarded: each read that misses
	 * makes the CPU fill one 16-byte line. Afterwards the whole CAR range
	 * (_car_region_start.._car_region_end from car.ld) is in L1. With an
	 * 8 KiB L1 (486DX/DX2) that is the whole cache, with 16 KiB (DX4,
	 * Am5x86) half of it. */
	movl	$_car_region_start, %esi
	movl	$_car_region_end, %ecx
	sub	%esi, %ecx
	shr	$2, %ecx
	cld
	rep	lodsl	/* cld? DF=0 */

	post_code(POST_486_CAR_LOCKED)

	/* Enable Cache-as-RAM mode by disabling cache. */
	/* CD = 1 and NW = 1: no more line fills, no write-through, hits are
	 * still served by L1. The filled lines can no longer be evicted, so
	 * they behave as 8 KiB of RAM. */
	movl	%cr0, %eax
	orl	$(CR0_CacheDisable | CR0_NoWriteThrough), %eax
	movl	%eax, %cr0

	/* NOW in cache as RAM */

	post_code(POST_486_CAR_DRAM_OFF)

#if CONFIG(NORTHBRIDGE_ALI_M1489)
	/* disable L1 /ken generation in chipset */
	movb	$0x16, %al
	outb	%al, $0x22
	movb	$0x30, %al
	outb	%al, $0x23

#elif CONFIG(NORTHBRIDGE_UMC_UM8881)
	/* disable RAM (L1 CAR is now filled) */
	/* Back to "no DRAM banks"; romstage sizes the real DRAM later. From
	 * here an access outside the CAR range reads or writes nothing, so
	 * stack and data must stay inside it. */
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x52), $0x00)
	pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x53), $0x00)

#elif CONFIG(NORTHBRIDGE_OPTI_VIPERM)
	/* No DRAM bank decoded any more (82C557M index 13h = 88h, the IBM
	 * init value): a CAR line that is not in L1 now reads back wrong. */
	movb	$0x13, %al
	outb	%al, $0x22
	movb	$0x88, %al
	outb	%al, $0x24

	/* CAR self-test: every dword holds its address, then its complement. */
	xorl	%ebx, %ebx
.Lcar_test_pass:
	movl	$_car_region_start, %edi
.Lcar_test_write:
	movl	%edi, %eax
	xorl	%ebx, %eax
	movl	%eax, (%edi)
	addl	$4, %edi
	cmpl	$_car_region_end, %edi
	jb	.Lcar_test_write
	movl	$_car_region_start, %edi
.Lcar_test_read:
	movl	%edi, %eax
	xorl	%ebx, %eax
	cmpl	%eax, (%edi)
	jne	.Lcar_test_fail
	addl	$4, %edi
	cmpl	$_car_region_end, %edi
	jb	.Lcar_test_read
	xorl	$0xffffffff, %ebx
	jnz	.Lcar_test_pass
	post_code(POST_486_CAR_TEST_OK)
	jmp	.Lcar_test_done
.Lcar_test_fail:
	post_code(POST_486_CAR_TEST_FAIL)
	hlt
	jmp	.Lcar_test_fail
.Lcar_test_done:
#endif



/* Disabled (#if 0): chipset register values copied from a Linux dump,
 * kept for reference. Not built. */
#if 0
/*
FORCE table

linux
50: fb a4 e6 8f 03 c1 ff 0f ff 0f 0d 2c e0 30 00 ff
after this
50: fb a4 e6 8f 01 00 ff 0f ff 0f 0d 00 e0 30 00 ff
*/
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x04), $0x0f)
# pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x50), $0xfb)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x51), $0xa4)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x52), $0xe6)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x53), $0x8f)
//pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x54), $0x03)
//pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x55), $0xc1)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x56), $0xff)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x57), $0x0f)
//pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x58), $0xff)
//pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x59), $0x0f)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x5a), $(0x0d /*| 0x40*/))	//?force L2 hit
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x5b), $0x2c)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x5c), $0xe0)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x5d), $0x30)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x60), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x61), $0xf4)
pci_write(UMC_PCI_BDFR(0, 0x10, 0, 0x62), $0x01)
////////////////////////////
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x04), $0x07)	//0xf with special cycle
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x40), $0x04)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x41), $0x42)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x42), $0x08)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x43), $0x9a)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x44), $0xbc)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x45), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x46), $0x1f)	//disabled INTC
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x47), $0x7f)
# pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x50), $0x80 /*| 1*/)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x51), $0x40 /*| 1*/)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x56), $0x01)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x57), $0x30)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x70), $0x6f)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x71), $0xff)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x72), $0x01)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x90), $0x02)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x91), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0x92), $0x1f)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa0), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa1), $0x80)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa2), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa3), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa4), $0x81)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa5), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa6), $0x00)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa7), $0xef)
pci_write(UMC_PCI_BDFR(0, 0x12, 0, 0xa8), $0x28)
#endif


post_code(POST_486_CAR_CLEAR)


#if 0	/* ========================= PCI DUMP ================ */
	movb	$0x0, %cl
pci_dump_loop2:
	movw	$0xcf8, %dx
	movl	$ UMC_PCI_BDFR(0, 0x10, 0, 0), %eax
	movb	%cl, %al
	outl	%eax, %dx

	movw	$0xcfc, %dx
	and	$0x3, %al
	add	%al, %dl
	inb	%dx, %al

	send_uart(UART_BASE)

	inc	%cl

	cmp	$0x80, %cl
	jne	pci_dump_loop2
#endif /* PCI DUMP */

	/*
		now no write goes outside (to RAM) if in hit region
		cache lines will not fill
		NW must be 1, or else CPU will keeps writing to RAM even on hits
	*/

	/* Clear the cache memory region. There should not be any traffic to RAM */
	/* rep stosl = store EAX (0) to ECX dwords at ES:EDI, upwards. This
	 * also zeroes the bootblock .bss, which car.ld puts inside CAR. */
	movl	$_car_region_start, %edi
	movl	$_car_region_end, %ecx
	sub	%edi, %ecx
	shr	$2, %ecx

	xorl	%eax, %eax
	/* movl	$0x12345678, %eax */

	cld
	rep	stosl	/* cld? DF=0 */

	post_code(POST_486_CAR_STACK)

	//NOTICE _ecar_stack moved lower in linker script
	/* _ecar_stack = top of the car_stack region (car.ld). The stack is
	 * the bottom 4 KiB of CAR (0x1A0000-0x1A1000 in this build) and grows
	 * down towards _car_region_start; nothing detects an overflow. */
	movl	$_ecar_stack, %esp

	/* Align the stack and keep aligned for call to bootblock_c_entry() */
	/* 16-byte aligned, minus 4, minus the 3 pushes below (12) = 16-byte
	 * aligned again at the call, as the i386 ABI of gcc expects. */
	and	$0xfffffff0, %esp	/*lower 4 bits always 0*/
	sub	$4, %esp		/*== push 4 bytes*/

	/*
		Restore the BIST result, should be still in EBP
		set in entry32 bootblock_protected_mode_entry

		no DX stored, could be still intact? unless outb/inb with dx
		can be compressed to eax (BIST != 0 failed -> halt)
	*/

	/* Arguments of bootblock_c_entry_bist(uint64_t base_timestamp,
	 * uint32_t bist), pushed right to left: bist, then the 64-bit
	 * timestamp as high dword and low dword. */
	pushl	%ebp

	/* Dummy timestamp - not implemented on 486 */
	pushl	$0
	pushl	$0


	//10:08000ff0

#if 0
	mov	%ss,%ax
	send_uart(UART_BASE)
	mov	%ah,%al
	send_uart(UART_BASE)

	mov	%esp,%eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)


	pushl	$0x42434445
	pop	%eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
#endif

//////

#if 0
	call	test
test:
	pop	%eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
	shr $0x8, %eax
	send_uart(UART_BASE)
#endif



#if 0
	//quick search debug block
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rdtsc
#endif


	#if 0
	post_code(/*POST_BOOTBLOCK_BEFORE_C_ENTRY*/ '8')
	#endif

/* before_c_entry: */
	/* C from here: src/cpu/486/bootblock.c -> bootblock_main_with_basetime()
	 * -> early chipset init, CBFS lookup, jump to romstage. */
	call	bootblock_c_entry_bist
	/* Never returns */
	/* Safety net: print 'H' and halt forever. */
.Lhlt:
	post_code(POST_DEAD_CODE)
	hlt
	jmp	.Lhlt

/* Unused leftover string (cpu/qemu-x86 uses it to load a "pagetables"
 * CBFS file); it only costs 11 bytes of flash. */
pagetables_name:
	.string "pagetables"
