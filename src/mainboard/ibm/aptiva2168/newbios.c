/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * Options of the setup utility (CMOS layout in newbios_cmos.h) that coreboot applies on the Aptiva 2168: the DRAM
 * timings of CHIPSET FEATURES (NB_VIPERM_FIELDS; Auto Configuration keeps romstage's, the IBM BIOS's for the bus clock)
 * and the PCI interrupt routing.
 */

#include <arch/io.h>
#include <arch/pirq_routing.h>
#include <commonlib/helpers.h>
#include <northbridge/opti/viperm/viperm.h>
#include <pc80/mc146818rtc.h>

#include <newbios_cmos.h>

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

/* Romstage, after the DRAM init: counts unfinished POSTs (SeaBIOS clears it at the end of POST) so that bad manual
   timings fall back to Auto after NB_BOOTFAIL_MAX tries. */
void newbios_chipset_options(void)
{
	static const struct nb_field fields[] = { NB_VIPERM_FIELDS };
	u8 fails = cmos_read(NB_CMOS_BOOTFAIL);

	cmos_write(fails < 0xff ? fails + 1 : fails, NB_CMOS_BOOTFAIL);
	if (!settings_valid() || setting(NB_CHIPSET_AUTO) || fails >= NB_BOOTFAIL_MAX)
		return;
	for (const struct nb_field *f = fields; f < fields + ARRAY_SIZE(fields); f++) {
		u8 v = setting(f->setting);
		if (v < f->count)
			viperm_write(f->reg, (viperm_read(f->reg) & ~f->mask) | f->value[v]);
	}
}

/* Ramstage PIRQ routing: links INTA#-INTD# set in setup keep their IRQ if the 82C558M can route it (5, 9-12, 14, 15,
   as setup offers), anything else is routed automatically. */
#define SETUP_PCI_IRQS 0xde20

void mainboard_pirq_preset(unsigned char *pirq)
{
	if (!settings_valid())
		return;
	for (int i = 0; i < 4; i++) {
		u8 irq = setting(NB_PCI_IRQ_A + i);
		pirq[i] = (SETUP_PCI_IRQS >> irq) & 1 ? irq : 0;
	}
}
