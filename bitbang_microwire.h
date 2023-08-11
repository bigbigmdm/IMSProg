/*  Copyright (C) 2005-2021 Mokrushin I.V. aka McMCC mcmcc@mail.ru
    A simple bitbang protocol for Microwire 8-pin serial EEPROMs
    (93XX devices). Support organization 8bit and 16bit(8bit emulation).

    bitbang_microwire.h

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */

#ifndef _BITBANG_MICROWIRE_H
#define _BITBANG_MICROWIRE_H

#if 0
extern unsigned char ORG; /* organization 0 = 8 bit and 1 = 16 bit */
#endif
extern unsigned char CLK;
extern unsigned char DO;
extern unsigned char DI;
extern unsigned char CSEL;

extern int org;
extern int mw_eepromsize;
extern int fix_addr_len;

struct MW_EEPROM {
	char *name;
	unsigned int size;
};

struct gpio_cmd {
	int (*gpio_setdir)(void);
	int (*gpio_setbits)(unsigned char bit);
	int (*gpio_getbits)(unsigned char *data);
};

void Erase_EEPROM_3wire(int size_eeprom);
int Read_EEPROM_3wire(unsigned char *buffer, int size_eeprom);
int Write_EEPROM_3wire(unsigned char *buffer, int size_eeprom);
int deviceSize_3wire(char *eepromname);
int Read_EEPROM_3wire_param(unsigned char *buffer, int start_addr, int block_size, int size_eeprom, unsigned char algorithm);
int Write_EEPROM_3wire_param(unsigned char *buffer, int start_addr, int block_size, int size_eeprom, unsigned char algorithm);

const static struct MW_EEPROM mw_eepromlist[] = {
	{ "93c06", 32 },
	{ "93c16", 64 },
	{ "93c46", 128 },
	{ "93c56", 256 },
	{ "93c66", 512 },
	{ "93c76", 1024 },
	{ "93c86", 2048 },
	{ "93c96", 4096 },
	{ 0, 0 }
};

#define MAX_MW_EEPROM_SIZE	4096

#endif /* _BITBANG_MICROWIRE_H */
/* End of [bitbang_microwire.h] package */
