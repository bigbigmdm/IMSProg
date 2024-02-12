/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * 2023-2024 Mikhail Medvedev <e-ink-reader@yandex.ru>
 * snorcmd_api.h
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
#ifndef __SNORCMD_API_H__
#define __SNORCMD_API_H__
#include "types.h"
int snor_read(unsigned char *buf, unsigned long from, unsigned long len);
int snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b);
int snor_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned int addr4b);
int snor_erase(unsigned long offs, unsigned long len);
//int snor_erase_param(unsigned long offs, unsigned long len, unsigned int sector_size, unsigned int n_sectors);
int full_erase_chip(void);
int snor_write(unsigned char *buf, unsigned long to, unsigned long len);
long snor_init(void);
void support_snor_list(void);
int snor_read_devid(u8 *rxbuf, int n_rx);
int s95_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int s95_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int s95_unprotect(void);
int s95_full_erase(void);
#endif /* __SNORCMD_API_H__ */
/* End of [snorcmd_api.h] package */
