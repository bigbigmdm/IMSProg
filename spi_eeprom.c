/*
 * Copyright (C) 2022 McMCC <mcmcc@mail.ru>
 * spi_eeprom.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timer.h"
#include "spi_eeprom.h"
#include "spi_controller.h"

extern unsigned int bsize;
extern char eepromname[12];
struct spi_eeprom seeprom_info;
int seepromsize = 0;
int spage_size = 0;

static void wait_ready(void)
{
	uint8_t data[1];

	while (1) {
		SPI_CONTROLLER_Chip_Select_Low();
		SPI_CONTROLLER_Write_One_Byte(SEEP_RDSR_CMD);
		SPI_CONTROLLER_Read_NByte(data, 1, SPI_CONTROLLER_SPEED_SINGLE);
		SPI_CONTROLLER_Chip_Select_High();

		if ((data[0] & 0x01) == 0) break;
		usleep(1);
	}
}

static void write_enable(void)
{
	uint8_t data[1];

	while (1) {
		SPI_CONTROLLER_Chip_Select_Low();
		SPI_CONTROLLER_Write_One_Byte(SEEP_WREN_CMD);
		SPI_CONTROLLER_Chip_Select_High();
		usleep(1);

		SPI_CONTROLLER_Chip_Select_Low();
		SPI_CONTROLLER_Write_One_Byte(SEEP_RDSR_CMD);
		SPI_CONTROLLER_Read_NByte(data, 1, SPI_CONTROLLER_SPEED_SINGLE);
		SPI_CONTROLLER_Chip_Select_High();

		if (data[0] == 0x02) break;
		usleep(1);
	}
}

static void eeprom_write_byte(struct spi_eeprom *dev, uint32_t address, uint8_t data)
{
	uint8_t buf[5];

	write_enable();

	buf[0] = SEEP_WRITE_CMD;
	if (dev->addr_bits == 9 && address > 0xFF)
		buf[0] = buf[0] | 0x08;

	SPI_CONTROLLER_Chip_Select_Low();
	if (dev->addr_bits > 16) {
		buf[1] = (address & 0xFF0000) >> 16;
		buf[2] = (address & 0xFF00) >> 8;
		buf[3] = (address & 0xFF);
		buf[4] = data;
		SPI_CONTROLLER_Write_NByte(buf, 5, SPI_CONTROLLER_SPEED_SINGLE);
	} else if (dev->addr_bits < 10) {
		buf[1] = (address & 0xFF);
		buf[2] = data;
		SPI_CONTROLLER_Write_NByte(buf, 3, SPI_CONTROLLER_SPEED_SINGLE);
	} else {
		buf[1] = (address & 0xFF00) >> 8;
		buf[2] = (address & 0xFF);
		buf[3] = data;
		SPI_CONTROLLER_Write_NByte(buf, 4, SPI_CONTROLLER_SPEED_SINGLE);
	}
	SPI_CONTROLLER_Chip_Select_High();

	wait_ready();
}

static void eeprom_write_page(struct spi_eeprom *dev, uint32_t address, int page_size, uint8_t *data)
{
	uint8_t buf[MAX_SEEP_PSIZE];
	uint8_t offs = 0;

	memset(buf, 0, sizeof(buf));

	buf[0] = SEEP_WRITE_CMD;
	if (dev->addr_bits == 9 && address > 0xFF)
		buf[0] = buf[0] | 0x08;

	if (dev->addr_bits > 16) {
		buf[1] = (address & 0xFF0000) >> 16;
		buf[2] = (address & 0xFF00) >> 8;
		buf[3] = (address & 0xFF);
		offs = 4;
	} else if (dev->addr_bits < 10) {
		buf[1] = (address & 0xFF);
		offs = 2;
	} else {
		buf[1] = (address & 0xFF00) >> 8;
		buf[2] = (address & 0xFF);
		offs = 3;
	}

	memcpy(&buf[offs], data, page_size);

	write_enable();

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_NByte(buf, offs + page_size, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();

	wait_ready();
}

static uint8_t eeprom_read_byte(struct spi_eeprom *dev, uint32_t address)
{
	uint8_t buf[4];
	uint8_t data;
	buf[0] = SEEP_READ_CMD;

	if (dev->addr_bits == 9 && address > 0xFF)
		buf[0] = buf[0] | 0x08;

	SPI_CONTROLLER_Chip_Select_Low();
	if (dev->addr_bits > 16) {
		buf[1] = (address & 0xFF0000) >> 16;
		buf[2] = (address & 0xFF00) >> 8;
		buf[3] = (address & 0xFF);
		SPI_CONTROLLER_Write_NByte(buf, 4, SPI_CONTROLLER_SPEED_SINGLE);
		SPI_CONTROLLER_Read_NByte(buf, 1, SPI_CONTROLLER_SPEED_SINGLE);
		data = buf[0];
	} else if (dev->addr_bits < 10) {
		buf[1] = (address & 0xFF);
		SPI_CONTROLLER_Write_NByte(buf, 2, SPI_CONTROLLER_SPEED_SINGLE);
		SPI_CONTROLLER_Read_NByte(buf, 1, SPI_CONTROLLER_SPEED_SINGLE);
		data = buf[0];
	} else {
		buf[1] = (address & 0xFF00) >> 8;
		buf[2] = (address & 0xFF);
		SPI_CONTROLLER_Write_NByte(buf, 3, SPI_CONTROLLER_SPEED_SINGLE);
		SPI_CONTROLLER_Read_NByte(buf, 1, SPI_CONTROLLER_SPEED_SINGLE);
		data = buf[0];
	}
	SPI_CONTROLLER_Chip_Select_High();

	return data;
}

int32_t parseSEEPsize(char *seepromname, struct spi_eeprom *seeprom)
{
	int i;

	for (i = 0; seepromlist[i].total_bytes; i++) {
		if (strstr(seepromlist[i].name, seepromname)) {
			memcpy(seeprom, &(seepromlist[i]), sizeof(struct spi_eeprom));
			return (seepromlist[i].total_bytes);
		}
	}

	return -1;
}

int spi_eeprom_read(unsigned char *buf, unsigned long from, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_SEEP_SIZE];
	uint32_t i;

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0, sizeof(ebuf));
	pbuf = ebuf;

	for (i = 0; i < seepromsize; i++) {
		pbuf[i] = eeprom_read_byte(&seeprom_info, i);
		if( timer_progress() )
		{
			printf("\bRead %d%% [%d] of [%d] bytes      ", 100 * i / seepromsize, i, seepromsize);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}
	memcpy(buf, pbuf + from, len);

	printf("Read 100%% [%lu] bytes from [%s] EEPROM address 0x%08lu\n", len, eepromname, from);
	timer_end();

	return (int)len;
}

int spi_eeprom_erase(unsigned long offs, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_SEEP_SIZE];
	uint32_t i;

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0xff, sizeof(ebuf));
	pbuf = ebuf;

	if (offs || len < seepromsize) {
		for (i = 0; i < seepromsize; i++)
			pbuf[i] = eeprom_read_byte(&seeprom_info, i);
		memset(pbuf + offs, 0xff, len);
	}

	for (i = 0; i < seepromsize; i++) {
		if (spage_size) {
			eeprom_write_page(&seeprom_info, i, spage_size, pbuf + i);
			i = (spage_size + i) - 1;
		} else
			eeprom_write_byte(&seeprom_info, i, pbuf[i]);
		if( timer_progress() )
		{
			printf("\bErase %d%% [%d] of [%d] bytes      ", 100 * i / seepromsize, i, seepromsize);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}

	printf("Erased 100%% [%lu] bytes of [%s] EEPROM address 0x%08lu\n", len, eepromname, offs);
	timer_end();

	return 0;
}

int spi_eeprom_write(unsigned char *buf, unsigned long to, unsigned long len)
{
	unsigned char *pbuf, ebuf[MAX_SEEP_SIZE];
	uint32_t i;

	if (len == 0)
		return -1;

	timer_start();
	memset(ebuf, 0xff, sizeof(ebuf));
	pbuf = ebuf;

	if (to || len < seepromsize) {
		for (i = 0; i < seepromsize; i++)
			pbuf[i] = eeprom_read_byte(&seeprom_info, i);
	}
	memcpy(pbuf + to, buf, len);

	for (i = 0; i < seepromsize; i++) {
		if (spage_size) {
			eeprom_write_page(&seeprom_info, i, spage_size, pbuf + i);
			i = (spage_size + i) - 1;
		} else
			eeprom_write_byte(&seeprom_info, i, pbuf[i]);
		if( timer_progress() )
		{
			printf("\bWritten %d%% [%d] of [%d] bytes      ", 100 * i / seepromsize, i, seepromsize);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}

	printf("Written 100%% [%lu] bytes to [%s] EEPROM address 0x%08lu\n", len, eepromname, to);
	timer_end();

	return (int)len;
}

long spi_eeprom_init(void)
{
	if (seepromsize <= 0) {
		printf("SPI EEPROM Not Detected!\n");
		return -1;
	}

	bsize = 1;

	printf("SPI EEPROM chip: %s, Size: %d bytes\n", eepromname, seepromsize);

	return (long)seepromsize;
}

void support_spi_eeprom_list(void)
{
	int i;

	printf("SPI EEPROM Support List:\n");
	for ( i = 0; i < (sizeof(seepromlist)/sizeof(struct spi_eeprom)); i++)
	{
		if (!seepromlist[i].total_bytes)
			break;
		printf("%03d. %s\n", i + 1, seepromlist[i].name);
	}
}
/* End of [spi_eeprom.c] package */
