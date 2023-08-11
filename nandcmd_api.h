/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * nandcmd_api.h
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
#ifndef __NANDCMD_API_H__
#define __NANDCMD_API_H__

int snand_read(unsigned char *buf, unsigned long from, unsigned long len);
int snand_erase(unsigned long offs, unsigned long len);
int snand_write(unsigned char *buf, unsigned long to, unsigned long len);
long snand_init(void);
void support_snand_list(void);

extern int ECC_fcheck;
extern int ECC_ignore;
extern int OOB_size;
extern int Skip_BAD_page;
extern unsigned char _ondie_ecc_flag;

#endif /* __NANDCMD_API_H__ */
/* End of [nandcmd_api.h] package */
