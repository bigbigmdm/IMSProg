/*
 * Copyright (C) 2022 McMCC <mcmcc@mail.ru>
 * spi_eeprom_api.h
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
#ifndef __SPI_EEPROM_API_H__
#define __SPI_EEPROM_API_H__

int spi_eeprom_read(unsigned char *buf, unsigned long from, unsigned long len);
int spi_eeprom_erase(unsigned long offs, unsigned long len);
int spi_eeprom_write(unsigned char *buf, unsigned long to, unsigned long len);
long spi_eeprom_init(void);
void support_spi_eeprom_list(void);

#endif /* __SPI_EEPROM_API_H__ */
/* End of [spi_eeprom_api.h] package */
