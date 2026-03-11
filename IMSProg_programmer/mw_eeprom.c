/*
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * mw_eeprom.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bitbang_microwire.h"
#include "ch341a_gpio.h"
#include "timer.h"

extern struct gpio_cmd bb_func;
extern char eepromname[12];
extern unsigned int bsize;


/*
               25xx  93xx
 PIN 22 - D7 - MISO  DO
 PIN 21 - D6 -
 PIN 20 - D5 - MOSI  DI
 PIN 19 - D4 -
 PIN 18 - D3 - CLK   CLK
 PIN 17 - D2 -
 PIN 16 - D1 -
 PIN 15 - D0 - CS    CS
*/

int mw_gpio_init(void)
{
	int ret = 0;

	CLK  = 1 << 3;
	DO   = 1 << 7;
	DI   = 1 << 5;
	CSEL = 1 << 0;

	bb_func.gpio_setdir  = ch341a_gpio_setdir;
	bb_func.gpio_setbits = ch341a_gpio_setbits;
	bb_func.gpio_getbits = ch341a_gpio_getbits;

	if(bb_func.gpio_setdir)
		ret = bb_func.gpio_setdir();
	else
		return -1;

	if(ret < 0)
		return -1;

	return 0;
}

/* End of [mw_eeprom.c] package */
