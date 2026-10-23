## SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_NORTHBRIDGE_OPTI_VIPERM),y)

bootblock-y += bootblock.c

romstage-y += romstage.c
romstage-y += raminit.c

postcar-y += postcar.c

ramstage-y += northbridge.c

endif
