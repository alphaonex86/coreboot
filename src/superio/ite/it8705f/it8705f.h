/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef SUPERIO_ITE_IT8705F_H
#define SUPERIO_ITE_IT8705F_H

/* Datasheet: IT8705F/AF Super I/O */

#define IT8705F_FDC  0x00 /* Floppy */
#define IT8705F_SP1  0x01 /* UART1 */
#define IT8705F_SP2  0x02 /* UART2 */
#define IT8705F_PP   0x03 /* Parallel port */
#define IT8705F_EC   0x04 /* Environment controller (hwmon) */
#define IT8705F_KBCK 0x05 /* PS/2 keyboard */
#define IT8705F_KBCM 0x06 /* PS/2 mouse */
#define IT8705F_GPIO 0x07 /* GPIO */
#define IT8705F_MIDI 0x08 /* MIDI port */
#define IT8705F_GAME 0x09 /* GAME port */
#define IT8705F_IR   0x0a /* Consumer IR */

#endif /* SUPERIO_ITE_IT8705F_H */
