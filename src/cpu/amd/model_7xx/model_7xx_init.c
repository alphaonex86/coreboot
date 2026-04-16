/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * AMD K7 Athlon/Duron CPU initialization
 * Ported from legacy coreboot cpu/k7/cpufixup.c
 */

#include <device/device.h>
#include <cpu/cpu.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/cache.h>
#include <console/console.h>

/* AMD K7 specific MSRs */
#define TOP_MEM_MSR		0xC001001A
#define TOP_MEM2_MSR		0xC001001D
#define SYSCFG_MSR		0xC0010010
#define HWCR_MSR		0xC0010015
#define IORR0_BASE_MSR		0xC0010016
#define IORR0_MASK_MSR		0xC0010017
#define IORR1_BASE_MSR		0xC0010018
#define IORR1_MASK_MSR		0xC0010019

/* SYSCFG MSR bits */
#define SYSCFG_MTRR_VAR_DRAM_EN	(1 << 20)
#define SYSCFG_MTRR_FIX_DRAM_MOD_EN	(1 << 19)
#define SYSCFG_MTRR_FIX_DRAM_EN	(1 << 18)
#define SYSCFG_SYS_ECC_EN		(1 << 15)

static void model_7xx_init(struct device *dev)
{
	msr_t msr;

	printk(BIOS_DEBUG, "AMD K7: initializing CPU\n");

	/* Turn on caching if we haven't already */
	enable_cache();

	/* Clear IORR registers */
	msr.lo = 0;
	msr.hi = 0;
	wrmsr(IORR0_BASE_MSR, msr);
	wrmsr(IORR0_MASK_MSR, msr);
	wrmsr(IORR1_BASE_MSR, msr);
	wrmsr(IORR1_MASK_MSR, msr);

	/* Enable MTRR variable DRAM support in SYSCFG */
	msr = rdmsr(SYSCFG_MSR);
	msr.lo |= SYSCFG_MTRR_VAR_DRAM_EN;
	wrmsr(SYSCFG_MSR, msr);

	x86_setup_mtrrs();
	x86_mtrr_check();
}

static struct device_operations cpu_dev_ops = {
	.init = model_7xx_init,
};

/*
 * AMD Athlon Processor Model 1 Revision Guide
 * AMD Athlon Processor Model 2 Revision Guide
 * AMD Duron Processor Revision Guide
 * AMD Athlon Processor Model 4 Revision Guide
 * AMD Athlon Processor Model 6 Revision Guide
 *
 * K7 family = 6 in CPUID
 */
static const struct cpu_device_id cpu_table[] = {
	{ X86_VENDOR_AMD, 0x0610, CPUID_EXACT_MATCH_MASK }, /* Athlon (Argon) A0 */
	{ X86_VENDOR_AMD, 0x0611, CPUID_EXACT_MATCH_MASK }, /* Athlon (Argon) A1 */
	{ X86_VENDOR_AMD, 0x0612, CPUID_EXACT_MATCH_MASK }, /* Athlon (Argon) A2 */
	{ X86_VENDOR_AMD, 0x0620, CPUID_EXACT_MATCH_MASK }, /* Athlon (Pluto/Orion) A0 */
	{ X86_VENDOR_AMD, 0x0621, CPUID_EXACT_MATCH_MASK }, /* Athlon (Pluto/Orion) A1 */
	{ X86_VENDOR_AMD, 0x0622, CPUID_EXACT_MATCH_MASK }, /* Athlon (Pluto/Orion) A2 */
	{ X86_VENDOR_AMD, 0x0630, CPUID_EXACT_MATCH_MASK }, /* Duron (Spitfire) A0 */
	{ X86_VENDOR_AMD, 0x0631, CPUID_EXACT_MATCH_MASK }, /* Duron (Spitfire) A2 */
	{ X86_VENDOR_AMD, 0x0640, CPUID_EXACT_MATCH_MASK }, /* Athlon (Thunderbird) A1 */
	{ X86_VENDOR_AMD, 0x0641, CPUID_EXACT_MATCH_MASK }, /* Athlon (Thunderbird) A2 */
	{ X86_VENDOR_AMD, 0x0642, CPUID_EXACT_MATCH_MASK }, /* Athlon (Thunderbird) A4-A8 */
	{ X86_VENDOR_AMD, 0x0643, CPUID_EXACT_MATCH_MASK }, /* Athlon (Thunderbird) A9 */
	{ X86_VENDOR_AMD, 0x0660, CPUID_EXACT_MATCH_MASK }, /* Athlon (Palomino) A0 */
	{ X86_VENDOR_AMD, 0x0661, CPUID_EXACT_MATCH_MASK }, /* Athlon (Palomino) A2 */
	{ X86_VENDOR_AMD, 0x0662, CPUID_EXACT_MATCH_MASK }, /* Athlon XP (Palomino) A5 */
	{ X86_VENDOR_AMD, 0x0670, CPUID_EXACT_MATCH_MASK }, /* Duron (Morgan) A0 */
	{ X86_VENDOR_AMD, 0x0671, CPUID_EXACT_MATCH_MASK }, /* Duron (Morgan) A1 */
	{ X86_VENDOR_AMD, 0x0680, CPUID_EXACT_MATCH_MASK }, /* Athlon XP (Thoroughbred) A0 */
	{ X86_VENDOR_AMD, 0x0681, CPUID_EXACT_MATCH_MASK }, /* Athlon XP (Thoroughbred) B0 */
	CPU_TABLE_END
};

static const struct cpu_driver driver __cpu_driver = {
	.ops      = &cpu_dev_ops,
	.id_table = cpu_table,
};
