## SPDX-License-Identifier: GPL-2.0-only

ifeq ($(CONFIG_NORTHBRIDGE_SIS_SIS735),y)

romstage-y += raminit.c
romstage-y += romstage.c
romstage-y += memmap.c
romstage-y += early_smbus.c

ramstage-y += northbridge.c
ramstage-y += memmap.c

endif
