/*
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * mw_eeprom_api.h
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
#ifndef __MW_EEPROM_API_H__
#define __MW_EEPROM_API_H__

//int mw_eeprom_read(unsigned char *buf, unsigned long from, unsigned long len);
//int mw_eeprom_erase(unsigned long offs, unsigned long len);
//int mw_eeprom_write(unsigned char *buf, unsigned long to, unsigned long len);
//long mw_init(void);
int mw_gpio_init(void);
//void support_mw_eeprom_list(void);

#endif /* __MW_EEPROM_API_H__ */
/* End of [mw_eeprom_api.h] package */
