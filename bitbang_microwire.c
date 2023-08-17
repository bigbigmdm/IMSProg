/*  Copyright (C) 2005-2021 Mokrushin I.V. aka McMCC mcmcc@mail.ru,
    (C) 2023 Mikhail Medvedev e-ink-reader@yandex.ru
    A simple bitbang protocol for Microwire 8-pin serial EEPROMs
    (93XX devices). Support organization 8bit and 16bit(8bit emulation).

    bitbang_microwire.c

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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bitbang_microwire.h"

struct gpio_cmd bb_func;

#if 0
unsigned char ORG = 0; /* organization 0 = 8 bit and 1 = 16 bit */
#endif
unsigned char CLK = 0;
unsigned char DO = 0;
unsigned char DI = 0;
unsigned char CSEL = 0;

int org = 0;
int mw_eepromsize = 0;
int fix_addr_len = 0;

static unsigned char data = 0;

static void delay_ms(int n)
{
#if 0 /* ifndef _WINDOWS */
	int i;
	for(i = 0; i < (n * 100000); i++);
#else
    usleep((unsigned int)n);
#endif
}

static void data_1()
{
	data = data | DI;
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static void data_0()
{
	data = data & (~DI);
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static void org_1()
{
#if 0
	data = data | ORG; /* 16bit */
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
#endif
}

static void org_0()
{
#if 0
	data = data & (~ORG); /* 8bit */
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
#endif
}

static void csel_1()
{
	data = data | CSEL;
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static void csel_0()
{
	data = data & (~CSEL);
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static void clock_1()
{
	data = data | CLK;
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static void clock_0()
{
	data = data & (~CLK);
	if (bb_func.gpio_setbits)
		bb_func.gpio_setbits(data);
}

static unsigned int get_data()
{
	unsigned char b = 0;
	if (bb_func.gpio_getbits)
		bb_func.gpio_getbits(&b);
	return ((b & DO) == DO);
}

static int convert_size(int eeprom_size)
{
	int k = 1;

	org_0();
	delay_ms(1);
	if (org)
	{
		org_1();
		delay_ms(1);
		k = 2;
	}
	eeprom_size = eeprom_size / k;

	return eeprom_size;
}

static void send_to_di(unsigned int val, int nbit)
{
    unsigned int b = 0;
    int i = 0;

	while (i < nbit)
	{
		b = val & (1 << ((nbit - i++) - 1));
		clock_0();
		if (b)
			data_1();
		else
			data_0();
		delay_ms(1);
		clock_1();
		delay_ms(1);
	}
}

static unsigned char get_from_do()
{
	unsigned char val = 0;
    unsigned int b = 0, i = 0;
	while (i < 8)
	{
		clock_0();
		delay_ms(1);
		b = get_data();
		clock_1();
		delay_ms(1);
		val |= (b << (7 - i++));
	}
	return val;
}

static void enable_write_3wire(int num_bit)
{
	csel_0();
	clock_0();
	delay_ms(1);
	data_1();
	csel_1();
	delay_ms(1);
	clock_1();
	delay_ms(1);

	send_to_di(3, 4);
	send_to_di(0, num_bit - 2);

	data_0();
	delay_ms(1);
	csel_0();
	delay_ms(1);
}

static void disable_write_3wire(int num_bit)
{
	csel_1();
	delay_ms(1);
	clock_0();
	delay_ms(1);
	data_1();
	delay_ms(1);
	clock_1();
	delay_ms(1);
	send_to_di(0, 4);
	send_to_di(0, num_bit - 2);
	csel_0();
	delay_ms(1);
}

static void chip_busy(void)
{
	printf("Error: Always BUSY! Communication problem...The broken microwire chip?\n");
}


void Erase_EEPROM_3wire_param(unsigned char algorithm)
{
    int i, num_bit;

    num_bit = algorithm & 0x0f;

    enable_write_3wire(num_bit);
    csel_0();
    clock_0();
    delay_ms(1);
    data_1();
    csel_1();
    delay_ms(1);
    clock_1();
    delay_ms(1);

    send_to_di(2, 4);
    send_to_di(0, num_bit - 2);
    clock_0();
    data_0();
    delay_ms(1);
    csel_0();
    delay_ms(1);
    csel_1();
    delay_ms(1);
    clock_1();
    delay_ms(1);
    i = 0;
    while (!get_data() && i < 10000)
    {
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        i++;
    }

    if (i == 10000)
    {
        chip_busy();
        return;
    }

    csel_0();
    delay_ms(1);
    clock_0();
    delay_ms(1);
    clock_1();
    delay_ms(1);

    disable_write_3wire(num_bit);
}



int Read_EEPROM_3wire_param(unsigned char *buffer, int start_addr, int block_size, int size_eeprom, unsigned char algorithm)
{
    int address, num_bit, l;
    org = (algorithm & 0xf0) / 16;
    num_bit = algorithm & 0x0f;
    //num_bit = addr_nbits(__func__, size_eeprom);
    //printf("numbit=%x org=%x\n",num_bit,org);
    if (org == 1)
    {
        block_size = block_size / 2;
        start_addr = start_addr / 2;
        num_bit--;
    }
    size_eeprom = convert_size(size_eeprom);

    address = 0;

    for (l = start_addr; l < block_size + start_addr; l++)
    {
        csel_0();
        clock_0();
        delay_ms(1);
        data_1();
        csel_1();
        delay_ms(1);
        clock_1();
        delay_ms(1);

        send_to_di(2, 2);
        send_to_di((unsigned int)l, num_bit);

        data_0();
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        if (org == 0 ) buffer[address] = get_from_do();
        if (org == 1)
        {
            buffer[address + 1] = get_from_do();
            buffer[address] = get_from_do();
            address++;
        }
        csel_0();
        delay_ms(1);
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        //printf("\bRead %d%% [%d] of [%d] bytes      ", 100 * l / block_size, l, block_size);
        //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        //fflush(stdout);
        address++;
    }
    //printf("Read 100%% [%d] of [%d] bytes      \n", l, block_size);
    return 0;
}

int Write_EEPROM_3wire_param(unsigned char *buffer, int start_addr, int block_size, int size_eeprom, unsigned char algorithm)
{
    int i, l, address, num_bit;

    //num_bit = addr_nbits(__func__, size_eeprom);
    org = (algorithm & 0xf0) / 16;
    num_bit = algorithm & 0x0f;
    if (org == 1)
    {
        block_size = block_size / 2;
        start_addr = start_addr / 2;
        num_bit--;
    }
    size_eeprom = convert_size(size_eeprom);

    enable_write_3wire(num_bit);
    address = 0;

    for (l = start_addr; l < block_size + start_addr; l++)
    {
        csel_0();
        clock_0();
        delay_ms(1);
        data_1();
        csel_1();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        send_to_di(1, 2);
        send_to_di((unsigned int)l, num_bit);

        if (org == 0 ) send_to_di(buffer[address], 8);
        if (org == 1)
        {
            send_to_di(buffer[address +1], 8);
            send_to_di(buffer[address], 8);
            address++;
        }
        clock_0();
        data_0();
        delay_ms(1);
        csel_0();
        delay_ms(1);
        csel_1();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        i = 0;
        while (!get_data() && i < 10000)
        {
            clock_0();
            delay_ms(1);
            clock_1();
            delay_ms(1);
            i++;
        }
        if (i == 10000)
        {
            printf("\n");
            chip_busy();
            return -1;
        }

        csel_0();
        delay_ms(1);
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        //printf("\bWritten %d%% [%d] of [%d] bytes      ", 100 * l / block_size, l, block_size);
        //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        //fflush(stdout);
        address++;
    }
    //printf("Written 100%% [%d] of [%d] bytes      \n", l, block_size);
    disable_write_3wire(num_bit);

    return 0;
}
//static int addr_nbits(const char *func, int size)
//{
//	int i = 0;
//
//	if (fix_addr_len) {
//		printf("%s: Set address len %d bits\n", func, fix_addr_len);
//		return fix_addr_len;
//	}
//
//	switch (size) {
//		case 128: /* 93c46 */
//           i = org ? 6 : 7;
//			break;
//		case 256: /* 93c56 */
//		case 512: /* 93c66 */
//			i = org ? 8 : 9;
//			break;
//		case 1024: /* 93c76 */
//		case 2048: /* 93c86 */
//			i = org ? 10 : 11;
//			break;
//		case 4096: /* 93c96(not original name) */
//			i = org ? 12 : 13;
//			break;
//		default:
//			i = 6; /* 93c06 and 93c16(not original name) */
//			break;
//	}
//
//	printf("%s: Set address len %d bits\n", func, i);
//
//	return i;
//}

/*int Read_EEPROM_3wire(unsigned char *buffer, int size_eeprom)
{
    int address, num_bit, l;

    num_bit = addr_nbits(__func__, size_eeprom);
    size_eeprom = convert_size(size_eeprom);

    address = 0;

    for (l = 0; l < size_eeprom; l++)
    {
        csel_0();
        clock_0();
        delay_ms(1);
        data_1();
        csel_1();
        delay_ms(1);
        clock_1();
        delay_ms(1);

        send_to_di(2, 2);
        send_to_di(l, num_bit);

        data_0();
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        buffer[address] = get_from_do();
        if (org)
        {
            address++;
            buffer[address] = get_from_do();
        }
        csel_0();
        delay_ms(1);
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        printf("\bRead %d%% [%d] of [%d] bytes      ", 100 * l / size_eeprom, l, size_eeprom);
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        fflush(stdout);
        address++;
    }
    printf("Read 100%% [%d] of [%d] bytes      \n", l, size_eeprom);
    return 0;
}
*/
/*void Erase_EEPROM_3wire(int size_eeprom)
{
    int i, num_bit;

    num_bit = addr_nbits(__func__, size_eeprom);

    enable_write_3wire(num_bit);
    csel_0();
    clock_0();
    delay_ms(1);
    data_1();
    csel_1();
    delay_ms(1);
    clock_1();
    delay_ms(1);

    send_to_di(2, 4);
    send_to_di(0, num_bit - 2);
    clock_0();
    data_0();
    delay_ms(1);
    csel_0();
    delay_ms(1);
    csel_1();
    delay_ms(1);
    clock_1();
    delay_ms(1);
    i = 0;
    while (!get_data() && i < 10000)
    {
        clock_0();
        delay_ms(1);
        clock_1();
        delay_ms(1);
        i++;
    }

    if (i == 10000)
    {
        chip_busy();
        return;
    }

    csel_0();
    delay_ms(1);
    clock_0();
    delay_ms(1);
    clock_1();
    delay_ms(1);

    disable_write_3wire(num_bit);
}
*/
/*
int Write_EEPROM_3wire(unsigned char *buffer, int size_eeprom)
{
	int i, l, address, num_bit;

	num_bit = addr_nbits(__func__, size_eeprom);
	size_eeprom = convert_size(size_eeprom);

	enable_write_3wire(num_bit);
	address = 0;

	for (l = 0; l < size_eeprom; l++)
	{
		csel_0();
		clock_0();
		delay_ms(1);
		data_1();
		csel_1();
		delay_ms(1);
		clock_1();
		delay_ms(1);
		send_to_di(1, 2);
        send_to_di((unsigned int)l, num_bit);
		send_to_di(buffer[address], 8);
		if (org)
		{
			address++;
			send_to_di(buffer[address], 8);
		}
		clock_0();
		data_0();
		delay_ms(1);
		csel_0();
		delay_ms(1);
		csel_1();
		delay_ms(1);
		clock_1();
		delay_ms(1);
		i = 0;
		while (!get_data() && i < 10000)
		{
			clock_0();
			delay_ms(1);
			clock_1();
			delay_ms(1);
			i++;
		}
		if (i == 10000)
		{
			printf("\n");
			chip_busy();
			return -1;
		}

		csel_0();
		delay_ms(1);
		clock_0();
		delay_ms(1);
		clock_1();
		delay_ms(1);
		printf("\bWritten %d%% [%d] of [%d] bytes      ", 100 * l / size_eeprom, l, size_eeprom);
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
		fflush(stdout);
		address++;
	}
	printf("Written 100%% [%d] of [%d] bytes      \n", l, size_eeprom);
	disable_write_3wire(num_bit);

	return 0;
}

int deviceSize_3wire(char *eepromname)
{
	int i;

	for (i = 0; mw_eepromlist[i].size; i++) {
		if (strstr(mw_eepromlist[i].name, eepromname)) {
            return ((int)mw_eepromlist[i].size);
		}
	}

	return -1;
}
*/
/* End of [bitbang_microwire.c] package */
