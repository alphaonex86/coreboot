/* SPDX-License-Identifier: GPL-2.0-only */

#include <lib.h>
#include <stdint.h>

/* Undoes the x86 BCJ filter of xz/liblzma (start offset 0) that newBIOS applies to payload segments
   before LZMA: absolute CALL/JMP targets back to relative. Same algorithm as tools/bcj_x86.py. */

static int msbyte(uint8_t b)
{
	return b == 0x00 || b == 0xff;
}

void bcj_x86_decode(uint8_t *buf, size_t size)
{
	static const uint8_t allowed[8] = { 1, 1, 1, 0, 1, 0, 0, 0 };
	static const uint8_t bitnum[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
	uint32_t prev_mask = 0;
	size_t prev_pos = (size_t)-1, i = 0;

	while (i + 4 < size) {
		if ((buf[i] & 0xfe) != 0xe8) {
			i++;
			continue;
		}
		size_t dist = i - prev_pos;
		if (dist > 3) {
			prev_mask = 0;
		} else {
			prev_mask = (prev_mask << (dist - 1)) & 7;
			if (prev_mask && (!allowed[prev_mask] || msbyte(buf[i + 4 - bitnum[prev_mask]]))) {
				prev_pos = i++;
				prev_mask = (prev_mask << 1) | 1;
				continue;
			}
		}
		prev_pos = i;
		if (!msbyte(buf[i + 4])) {
			prev_mask = (prev_mask << 1) | 1;
			i++;
			continue;
		}
		uint32_t src = buf[i + 1] | buf[i + 2] << 8 | buf[i + 3] << 16 | (uint32_t)buf[i + 4] << 24;
		uint32_t dest;
		for (;;) {
			dest = src - (uint32_t)(i + 5);
			if (!prev_mask)
				break;
			unsigned int j = bitnum[prev_mask] * 8;
			if (!msbyte(dest >> (24 - j)))
				break;
			src = dest ^ ((1u << (32 - j)) - 1);
		}
		dest &= 0x01ffffff;
		dest |= 0u - (dest & 0x01000000);
		buf[i + 1] = dest;
		buf[i + 2] = dest >> 8;
		buf[i + 3] = dest >> 16;
		buf[i + 4] = dest >> 24;
		i += 5;
	}
}
