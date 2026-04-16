## SPDX-License-Identifier: GPL-2.0-only

ifeq ($(CONFIG_SOUTHBRIDGE_SIS_SIS85C503),y)

bootblock-y += bootblock.c

ramstage-y += sis85c503.c
ramstage-y += ide.c
ramstage-y += usb.c

endif
