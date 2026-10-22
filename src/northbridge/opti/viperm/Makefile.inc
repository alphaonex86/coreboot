## SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_NORTHBRIDGE_OPTI_VIPERM),y)

romstage-y += romstage.c
romstage-y += memmap.c

ramstage-y += northbridge.c
ramstage-y += memmap.c

endif
