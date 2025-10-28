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

int snor_wait_ready(int sleep_ms);
int snor_read(unsigned char *buf, unsigned long from, unsigned long len);
int snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b);
int snor_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned int addr4b);
int snor_erase(unsigned long offs, unsigned long len);
//int snor_erase_param(unsigned long offs, unsigned long len, unsigned int sector_size, unsigned int n_sectors);
int full_erase_chip(void);
int snor_block_erase(unsigned int sector_number, unsigned int blockSize, u8 addr4b);
int snor_write(unsigned char *buf, unsigned long to, unsigned long len);
long snor_init(void);
void support_snor_list(void);
int snor_read_devid(u8 *rxbuf, int n_rx);
int s95_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int s95_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int s95_unprotect(void);
int s95_full_erase(void);
int at45_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int at45_write_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm);
int at45_full_erase(void);
int at45_sector_erase(unsigned int sectorNumber, unsigned int pageSize);
int at45_read_sr(u8 *val);
int nand_page_read(unsigned char *buf, unsigned int sector_size, u32 sector_number);
int nand_block_read(unsigned char *buf, unsigned int page_size, u32 block_number, unsigned int pages_per_block);
int nand_wait_ready(int sleep_ms);
int nand_read_devid(u8 *rxbuf, int n_rx);
int nand_block_erase(unsigned int sector_number, unsigned int blockSize);
void nand_write_enable(void);
void nand_unprotect(void);
int nand_page_write(unsigned char *buf, unsigned int page_size, u32 sector_number);
int nand_block_write(unsigned char *buf, unsigned int page_size, u32 block_number, unsigned int pages_per_block);
void nand_ECCEnable(void);
int nand_checkBadBlock(u32 blockNo, u32 sectSize, u32 blockPerSector);
#endif /* __SNORCMD_API_H__ */
/* End of [snorcmd_api.h] package */
