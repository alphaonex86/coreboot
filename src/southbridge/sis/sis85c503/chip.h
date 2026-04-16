/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SOUTHBRIDGE_SIS_SIS85C503_CHIP_H
#define SOUTHBRIDGE_SIS_SIS85C503_CHIP_H

#include <types.h>

struct southbridge_sis_sis85c503_config {
	bool ide_enable_primary;
	bool ide_enable_secondary;
};

#endif /* SOUTHBRIDGE_SIS_SIS85C503_CHIP_H */
