/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * flashcmd_api.c
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
#include "flashcmd_api.h"

#ifdef EEPROM_SUPPORT
#define __EEPROM___	"or EEPROM"
extern int eepromsize;
extern int mw_eepromsize;
extern int seepromsize;
#else
#define __EEPROM___	""
#endif

long flash_cmd_init(struct flash_cmd *cmd)
{
	long flen = -1;

#ifdef EEPROM_SUPPORT
	if ((eepromsize <= 0) && (mw_eepromsize <= 0) && (seepromsize <= 0)) {
#endif
		if ((flen = snand_init()) > 0) {
			cmd->flash_erase = snand_erase;
			cmd->flash_write = snand_write;
			cmd->flash_read  = snand_read;
		} else if ((flen = snor_init()) > 0) {
			cmd->flash_erase = snor_erase;
			cmd->flash_write = snor_write;
			cmd->flash_read  = snor_read;
		}
#ifdef EEPROM_SUPPORT
	} else if ((eepromsize > 0) || (mw_eepromsize > 0) || (seepromsize > 0)) {
		if ((eepromsize > 0) && (flen = i2c_init()) > 0) {
			cmd->flash_erase = i2c_eeprom_erase;
			cmd->flash_write = i2c_eeprom_write;
			cmd->flash_read  = i2c_eeprom_read;
		} else if ((mw_eepromsize > 0) && (flen = mw_init()) > 0) {
			cmd->flash_erase = mw_eeprom_erase;
			cmd->flash_write = mw_eeprom_write;
			cmd->flash_read  = mw_eeprom_read;
		} else if ((seepromsize > 0) && (flen = spi_eeprom_init()) > 0) {
			cmd->flash_erase = spi_eeprom_erase;
			cmd->flash_write = spi_eeprom_write;
			cmd->flash_read  = spi_eeprom_read;
		}
	}
#endif
	else
		printf("\nFlash" __EEPROM___ " not found!!!!\n\n");

	return flen;
}

void support_flash_list(void)
{
	support_snand_list();
	printf("\n");
	support_snor_list();
#ifdef EEPROM_SUPPORT
	printf("\n");
	support_i2c_eeprom_list();
	printf("\n");
	support_mw_eeprom_list();
	printf("\n");
	support_spi_eeprom_list();
#endif
}
/* End of [flashcmd.c] package */
