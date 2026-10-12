/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __X86_POST_CODE_H__
#define __X86_POST_CODE_H__

#include <commonlib/console/post_codes.h>
#include <cpu/486/newbios_post.h>

#if CONFIG(POST_IO) && !(ENV_BOOTBLOCK && CONFIG(NO_EARLY_BOOTBLOCK_POSTCODES))

/* The POST code on port 80h, for a POST card (pc2005 sent these to COM1). */
#define post_init
#define post_code(value)	\
	movb	$value, %al;	\
	outb	%al, $CONFIG_POST_IO_PORT

#else
#define post_init
#define post_code(value)
#endif

#endif /* __X86_POST_CODE_H__ */
