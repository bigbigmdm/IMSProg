/*
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * ch341a_gpio.h
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
#ifndef __CH341A_GPIO_H__
#define __CH341A_GPIO_H__

#include <stdint.h>

int ch341a_gpio_setdir(void);
int ch341a_gpio_setbits(uint8_t bits);
int ch341a_gpio_getbits(uint8_t *data);

#endif /* __CH341A_GPIO_H__ */
/* End of [ch341a_gpio.h] package */
