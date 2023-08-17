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

/*int mw_eeprom_read(unsigned char *buf, unsigned long from, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_MW_EEPROM_SIZE];

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0, sizeof(ebuf));
	pbuf = ebuf;

	Read_EEPROM_3wire(pbuf, mw_eepromsize);
	memcpy(buf, pbuf + from, len);

	printf("Read [%lu] bytes from [%s] EEPROM address 0x%08lu\n", len, eepromname, from);
	timer_end();

	return (int)len;
}
*/
/*int mw_eeprom_erase(unsigned long offs, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_MW_EEPROM_SIZE];

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0xff, sizeof(ebuf));
	pbuf = ebuf;

	if (offs || len < mw_eepromsize) {
		Read_EEPROM_3wire(pbuf, mw_eepromsize);
		memset(pbuf + offs, 0xff, len);
	}

	Erase_EEPROM_3wire(mw_eepromsize);

	if (offs || len < mw_eepromsize) {
		if (Write_EEPROM_3wire(pbuf, mw_eepromsize) < 0) {
			printf("Failed to erase [%lu] bytes of [%s] EEPROM address 0x%08lu\n", len, eepromname, offs);
			return -1;
		}
	}

	printf("Erased [%lu] bytes of [%s] EEPROM address 0x%08lu\n", len, eepromname, offs);
	timer_end();

	return 0;
}

int mw_eeprom_write(unsigned char *buf, unsigned long to, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_MW_EEPROM_SIZE];

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0xff, sizeof(ebuf));
	pbuf = ebuf;

	if (to || len < mw_eepromsize) {
		Read_EEPROM_3wire(pbuf, mw_eepromsize);
	}
	memcpy(pbuf + to, buf, len);

	Erase_EEPROM_3wire(mw_eepromsize);

	if (Write_EEPROM_3wire(pbuf, mw_eepromsize) < 0) {
		printf("Failed to write [%lu] bytes of [%s] EEPROM address 0x%08lu\n", len, eepromname, to);
		return -1;
	}

	printf("Wrote [%lu] bytes to [%s] EEPROM address 0x%08lu\n", len, eepromname, to);
	timer_end();

	return (int)len;
}
*/
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

/*static char *__itoa(int a)
{
	char *p, tmp[32];
	p = tmp;
	snprintf(p, sizeof(tmp), "%d", a);
	return p;
}
*/

/*long mw_init(void)
{
	if (mw_eepromsize <= 0) {
		printf("Microwire EEPROM Not Detected!\n");
		return -1;
	}

	if (mw_gpio_init() < 0)
		return -1;

	bsize = 1;

	printf("Microwire EEPROM chip: %s, Size: %d bytes, Org: %d bits, fix addr len: %s\n", eepromname, mw_eepromsize / (org ? 2 : 1),
			org ? 16 : 8, fix_addr_len ? __itoa(fix_addr_len) : "Auto");

	return (long)mw_eepromsize;
}
*/
/*void support_mw_eeprom_list(void)
{
	int i;

	printf("Microwire EEPROM Support List:\n");
	for ( i = 0; i < (sizeof(mw_eepromlist)/sizeof(struct MW_EEPROM)); i++)
	{
		if (!mw_eepromlist[i].size)
			break;
		printf("%03d. %s\n", i + 1, mw_eepromlist[i].name);
	}
}
*/
/* End of [mw_eeprom.c] package */
