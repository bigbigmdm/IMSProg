/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * flashcmd_api.h
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
#ifndef __FLASHCMD_API_H__
#define __FLASHCMD_API_H__

#include "nandcmd_api.h"
#include "snorcmd_api.h"
#ifdef EEPROM_SUPPORT
#include "i2c_eeprom_api.h"
#include "mw_eeprom_api.h"
#include "spi_eeprom_api.h"
#endif

struct flash_cmd {
	int (*flash_read)(unsigned char *buf, unsigned long from, unsigned long len);
	int (*flash_erase)(unsigned long offs, unsigned long len);
	int (*flash_write)(unsigned char *buf, unsigned long to, unsigned long len);
};

long flash_cmd_init(struct flash_cmd *cmd);
void support_flash_list(void);

#endif /* __FLASHCMD_API_H__ */
/* End of [flashcmd_api.h] package */
