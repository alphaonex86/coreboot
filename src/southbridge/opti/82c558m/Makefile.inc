## SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_SOUTHBRIDGE_OPTI_82C558M),y)

bootblock-y += bootblock.c

romstage-y += early_init.c

ramstage-y += 82c558m.c
ramstage-$(CONFIG_GENERATE_PIRQ_TABLE) += pirq.c

endif
