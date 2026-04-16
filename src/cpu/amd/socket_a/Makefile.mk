## SPDX-License-Identifier: GPL-2.0-only

subdirs-y += ../model_7xx

bootblock-y += ../../intel/car/p3/cache_as_ram.S
bootblock-y += ../../intel/car/bootblock.c
postcar-y += ../../intel/car/p4-netburst/exit_car.S
romstage-y += ../../intel/car/romstage.c
