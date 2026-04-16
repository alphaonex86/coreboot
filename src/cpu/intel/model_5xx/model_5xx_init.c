/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/device.h>
#include <cpu/cpu.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/cache.h>

static void model_5xx_init(struct device *dev)
{
	/* Turn on caching if we haven't already */
	enable_cache();
	x86_setup_mtrrs();
	x86_mtrr_check();
}

static struct device_operations cpu_dev_ops = {
	.init = model_5xx_init,
};

/*
 * Intel Pentium Processor Specification Update
 * Intel Pentium MMX Processor Specification Update
 */
static const struct cpu_device_id cpu_table[] = {
	{ X86_VENDOR_INTEL, 0x0510, CPUID_EXACT_MATCH_MASK }, /* P5 A1 */
	{ X86_VENDOR_INTEL, 0x0512, CPUID_EXACT_MATCH_MASK }, /* P5 B1 */
	{ X86_VENDOR_INTEL, 0x0513, CPUID_EXACT_MATCH_MASK }, /* P5 B2 */
	{ X86_VENDOR_INTEL, 0x0517, CPUID_EXACT_MATCH_MASK }, /* P5 C2 */
	{ X86_VENDOR_INTEL, 0x0519, CPUID_EXACT_MATCH_MASK }, /* P5 D0 */
	{ X86_VENDOR_INTEL, 0x0520, CPUID_EXACT_MATCH_MASK }, /* P54C B1 */
	{ X86_VENDOR_INTEL, 0x0521, CPUID_EXACT_MATCH_MASK }, /* P54C B3 */
	{ X86_VENDOR_INTEL, 0x0522, CPUID_EXACT_MATCH_MASK }, /* P54C B5 */
	{ X86_VENDOR_INTEL, 0x0524, CPUID_EXACT_MATCH_MASK }, /* P54C C2/mA1 */
	{ X86_VENDOR_INTEL, 0x0525, CPUID_EXACT_MATCH_MASK }, /* P54C C3/mA4 */
	{ X86_VENDOR_INTEL, 0x0526, CPUID_EXACT_MATCH_MASK }, /* P54C D0 */
	{ X86_VENDOR_INTEL, 0x0540, CPUID_EXACT_MATCH_MASK }, /* P54C (P24T Overdrive) */
	{ X86_VENDOR_INTEL, 0x0543, CPUID_EXACT_MATCH_MASK }, /* P54C (P24T Overdrive) */
	{ X86_VENDOR_INTEL, 0x0544, CPUID_EXACT_MATCH_MASK }, /* P55C (Pentium MMX) A3 */
	{ X86_VENDOR_INTEL, 0x0546, CPUID_EXACT_MATCH_MASK }, /* P55C (Pentium MMX) B1 */
	{ X86_VENDOR_INTEL, 0x0581, CPUID_EXACT_MATCH_MASK }, /* Quark SoC X1000 */
	CPU_TABLE_END
};

static const struct cpu_driver driver __cpu_driver = {
	.ops      = &cpu_dev_ops,
	.id_table = cpu_table,
};
