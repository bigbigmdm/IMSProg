/*
 * Copyright (C) 2022 McMCC <mcmcc@mail.ru>
 * spi_eeprom.h
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
#ifndef __SPI_EEPROM_H__
#define __SPI_EEPROM_H__

#define SEEP_WREN_CMD		0x06 /* Set Write Enable Instruction */
#define SEEP_WRDI_CMD		0x04 /* Reset Write Enable Instruction */
#define SEEP_RDSR_CMD		0x05 /* Read Status Register */
#define SEEP_WRSR_CMD		0x01 /* Write Status Register */
#define SEEP_READ_CMD		0x03 /* Read Data from Memory Array */
#define SEEP_WRITE_CMD		0x02 /* Write Data to Memory Array */

struct spi_eeprom
{
	char *name;
	uint32_t total_bytes; /* EEPROM total memory size */
	uint8_t addr_bits;    /* Number of address bit */
};

const static struct spi_eeprom seepromlist[] = {
	{  "25010",    128,  7 },
	{  "25020",    256,  8 },
	{  "25040",    512,  9 },
	{  "25080",   1024, 10 },
	{  "25160",   2048, 11 },
	{  "25320",   4096, 12 },
	{  "25640",   8192, 13 },
	{  "25128",  16384, 14 },
	{  "25256",  32768, 15 },
	{  "25512",  65536, 16 },
	{ "251024", 131072, 17 },
	{        0,      0,  0 }
};

#define MAX_SEEP_SIZE		131072
#define MAX_SEEP_PSIZE		1024

int32_t parseSEEPsize(char *seepromname, struct spi_eeprom *seeprom);

#endif /* __SPI_EEPROM_H__ */
/* End of [spi_eeprom.h] package */
