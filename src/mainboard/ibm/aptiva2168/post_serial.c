/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/io.h>
#include <console/console.h>

#include "post_serial.h"

void mainboard_post(uint8_t value)
{
	post_serial(value);
}
