/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Options of the newBIOS setup utility (CMOS layout in newbios_cmos.h) that coreboot applies:
 * chipset features, external cache and PCI interrupt routing. The register fields are the ones the
 * original Award BIOS programs for the same setup values (found with newBIOS/test/oracle).
 */

#define __SIMPLE_DEVICE__

#include <arch/cpu.h>
#include <arch/io.h>
#include <arch/pirq_routing.h>
#include <console/console.h>
#include <device/pci_ops.h>
#include <pc80/mc146818rtc.h>
#include <northbridge/umc/um8881/reg_init.h>

#include "newbios_cmos.h"

static int settings_valid(void)
{
	u16 sum = 0;

	for (int i = NB_CMOS_BASE; i <= NB_CMOS_SIG; i++)
		sum += cmos_read(i);
	return cmos_read(NB_CMOS_SIG) == NB_SIG_VALUE
		&& sum == (cmos_read(NB_CMOS_SUM) << 8 | cmos_read(NB_CMOS_SUM + 1));
}

static u8 setting(int index)
{
	return NB_CMOS_GET(cmos_read, index);
}

struct field {
	u8 setting;
	pci_devfn_t dev;
	u8 reg, mask;
	u8 count, value[4];   /* register field for setting value 0..count-1 */
};

/* Not timings: applied with Auto Configuration and after unfinished POSTs too, like the original. */
static const struct field caching[] = {
	{ NB_SYSBIOS_CACHE,     HOST_BRIDGE, 0x55, 0x20, 2, { 0x00, 0x20 } },
	{ NB_VIDBIOS_CACHE,     HOST_BRIDGE, 0x55, 0x01, 2, { 0x00, 0x01 } },
};

static const struct field chipset[] = {
	{ NB_DRAM_READ_WS,      HOST_BRIDGE, 0x51, 0xc0, 4, { 0x00, 0x40, 0x80, 0xc0 } },	/* 3..0 WS */
	{ NB_DRAM_WRITE_WS,     HOST_BRIDGE, 0x51, 0x30, 4, { 0x00, 0x10, 0x20, 0x30 } },
	{ NB_L2_WS,             HOST_BRIDGE, 0x50, 0x30, 4, { 0x00, 0x10, 0x20, 0x30 } },
	{ NB_HOST_PCI_POST_WS,  HOST_BRIDGE, 0x52, 0x80, 2, { 0x00, 0x80 } },	/* 1 WS, 0 WS */
	{ NB_HOST_PCI_BURST,    HOST_BRIDGE, 0x53, 0x80, 2, { 0x00, 0x80 } },
	{ NB_BURST_COPYBACK,    HOST_BRIDGE, 0x53, 0x40, 2, { 0x00, 0x40 } },
	{ NB_EARLY_CACHE_WRITE, HOST_BRIDGE, 0x5d, 0x80, 2, { 0x00, 0x80 } },
	{ NB_SLOW_REFRESH,      HOST_BRIDGE, 0x5d, 0x10, 2, { 0x00, 0x10 } },
	{ NB_EDO,               HOST_BRIDGE, 0x61, 0xc0, 2, { 0x00, 0xc0 } },
	{ NB_PCI_POSTED_WRITE,  ISA_BRIDGE,  0x40, 0x04, 2, { 0x00, 0x04 } },
	{ NB_DEVSEL,            ISA_BRIDGE,  0x40, 0x03, 3, { 0x01, 0x00, 0x02 } },	/* slow, medium, fast */
	{ NB_PCI_BUS_PARK,      ISA_BRIDGE,  0x41, 0x04, 2, { 0x04, 0x00 } },
	{ NB_PREEMPT_PCI,       ISA_BRIDGE,  0x46, 0x10, 2, { 0x10, 0x00 } },
	{ NB_KBC_CLOCK,         ISA_BRIDGE,  0x56, 0x0c, 4, { 0x00, 0x04, 0x08, 0x0c } },	/* 7.16 MHz, PCICLK/4 /3 /2 */
	{ NB_HOST_PCI_CLOCK,    ISA_BRIDGE,  0xa4, 0x03, 3, { 0x01, 0x02, 0x00 } },	/* 1:1, 1:2/3, 1:1/2 */
};

static void apply(const struct field *f, int count)
{
	for (; count--; f++) {
		u8 v = setting(f->setting);
		if (v < f->count)
			pci_update_config8(f->dev, f->reg, ~f->mask, f->value[v]);
	}
}

/* Romstage, after the chipset defaults: counts unfinished POSTs (SeaBIOS clears it at the end of
   POST) so that bad manual timings fall back to the defaults after NB_BOOTFAIL_MAX tries. */
void newbios_chipset_options(void)
{
	u8 fails = cmos_read(NB_CMOS_BOOTFAIL);

	cmos_write(fails < 0xff ? fails + 1 : fails, NB_CMOS_BOOTFAIL);
	if (!settings_valid())
		return;
	if (!setting(NB_L2_CACHE))	/* no wbinvd: the stack is still in the L1 (cache-as-RAM) */
		pci_update_config8(HOST_BRIDGE, HB50_L2_CFG_0, 0x3f, 0);
	apply(caching, ARRAY_SIZE(caching));
	if (setting(NB_SYSBIOS_CACHE)) {	/* as the original BIOS does with this option */
		pci_write_config8(HOST_BRIDGE, 0x58, 0x0f);
		pci_write_config8(HOST_BRIDGE, 0x59, 0x00);
	}
	if (setting(NB_CHIPSET_AUTO))
		return;
	if (fails >= NB_BOOTFAIL_MAX) {
		printk(BIOS_WARNING, "newBIOS: %u unfinished POSTs, chipset setup values not used\n",
		       fails);
		return;
	}
	apply(chipset, ARRAY_SIZE(chipset));
}

/* Core clock in MHz: dec + taken jnz is 4 clocks on 486-class CPUs, timed with PIT channel 2. */
static unsigned int cpu_mhz(void)
{
	u32 n = 200000;

	outb((inb(0x61) & ~0x02) | 0x01, 0x61);
	outb(0xb0, 0x43);
	outb(0xff, 0x42);
	outb(0xff, 0x42);
	asm volatile("1: decl %0; jnz 1b" : "+r"(n));
	outb(0x80, 0x43);
	u32 ticks = 0xffff - (inb(0x42) | inb(0x42) << 8);
	return ticks ? (954545 + ticks / 2) / ticks : 0;	/* 800000 clocks at 1.193182 MHz */
}

/* Core/bus clock ratio from the family and model (CPUID, else DX at reset); 1 if unknown, which
   gives the slowest timings. No DX2 runs above 80 MHz: faster ones are DX4s with a DX2 ID. */
static unsigned int cpu_multiplier(unsigned int mhz)
{
	u8 sig = cpu_have_cpuid() ? cpuid_eax(1) >> 4 : cmos_read(NB_CMOS_CPUSIG);

	if (sig >> 4 != 4)
		return 1;
	switch (sig & 0x0f) {
	case 3: case 5: case 7: return mhz > 85 ? 3 : 2;	/* DX2, SX2, DX2 write-back */
	case 8: case 9: return 3;		/* DX4 */
	case 14: case 15: return 4;		/* Am5x86 */
	}
	return 1;
}

/* Ramstage, code in DRAM with the caches on, so the clock can be measured: Auto Configuration
   (or the unfinished-POST fallback) sets the original BIOS's timings for the bus clock, from its own
   table (HB50/51, IBC56/A4 per bus clock class, verified in 86Box). */
void newbios_auto_timings(void)
{
	static const struct { u8 bus_below, hb50, hb51, ibc56, ibca4; } rows[] = {
		{ 29,  0x30, 0xf0, 0x00, 0x01 },	/* 16-25 MHz: L2 2-1-1-1, DRAM 0 WS, ISA PCICLK/3, PCI 1:1 */
		{ 37,  0x10, 0xa0, 0x01, 0x01 },	/* 33 MHz: 3-1-1-1, 1 WS, PCICLK/4, 1:1 */
		{ 45,  0x00, 0xa0, 0x00, 0x02 },	/* 40 MHz: 3-2-2-2, 1 WS, PCICLK/3, 1:2/3 */
		{ 255, 0x00, 0x00, 0x00, 0x00 },	/* 50 MHz: 3-2-2-2, 3 WS, PCICLK/3, 1:1/2 */
	};
	unsigned int mhz = cpu_mhz(), bus = mhz / cpu_multiplier(mhz), i;

	if (settings_valid() && !setting(NB_CHIPSET_AUTO)
	    && cmos_read(NB_CMOS_BOOTFAIL) - 1 < NB_BOOTFAIL_MAX)
		return;		/* the manual values of setup are in use */
	for (i = 0; bus >= rows[i].bus_below; i++)
		;
	printk(BIOS_INFO, "newBIOS: %u MHz bus, auto chipset timings %u\n", bus, i);
	pci_update_config8(HOST_BRIDGE, HB50_L2_CFG_0, ~0x30, rows[i].hb50);
	pci_update_config8(HOST_BRIDGE, HB51_L2_CFG_1, ~0xf0, rows[i].hb51);
	pci_update_config8(ISA_BRIDGE, IBC56_CLOCK, ~0x0f, rows[i].ibc56);
	pci_update_config8(ISA_BRIDGE, IBCA4_CLOCK_RATIO, ~0x03, rows[i].ibca4);
}

/* Ramstage PIRQ routing: links INTA#-INTD# set in setup keep their IRQ. */
void mainboard_pirq_preset(unsigned char *pirq)
{
	if (!settings_valid())
		return;
	for (int i = 0; i < 4; i++)
		pirq[i] = setting(NB_PCI_IRQ_A + i);
}
