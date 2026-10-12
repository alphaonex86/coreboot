/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Options of the newBIOS setup utility (CMOS layout in newbios_cmos.h) that coreboot applies:
 * chipset features, external cache and PCI interrupt routing. The register fields are the ones the
 * original Award BIOS programs for the same setup values (found with newBIOS/test/oracle).
 */

#define __SIMPLE_DEVICE__

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

/* Ramstage PIRQ routing: links INTA#-INTD# set in setup keep their IRQ. */
void mainboard_pirq_preset(unsigned char *pirq)
{
	if (!settings_valid())
		return;
	for (int i = 0; i < 4; i++)
		pirq[i] = setting(NB_PCI_IRQ_A + i);
}
