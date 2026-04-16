## SPDX-License-Identifier: GPL-2.0-only

bootblock-$(CONFIG_SUPERIO_SIS_SIS950) += early_serial.c
romstage-$(CONFIG_SUPERIO_SIS_SIS950) += early_serial.c
ramstage-$(CONFIG_SUPERIO_SIS_SIS950) += superio.c
