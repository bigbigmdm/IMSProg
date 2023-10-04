/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * spi_nor_flash.c
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

#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "spi_controller.h"
#include "snorcmd_api.h"
#include "types.h"
#include "timer.h"

#define min(a,b) (((a)<(b))?(a):(b))

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE			256

/* Flash opcodes. */
#define OPCODE_WREN			6	/* Write enable */
#define OPCODE_WRDI			4	/* Write disable */
#define OPCODE_RDSR			5	/* Read status register */
#define OPCODE_WRSR			1	/* Write status register */
#define OPCODE_READ			3	/* Read data bytes */
#define OPCODE_PP			2	/* Page program */
#define OPCODE_SE			0xD8	/* Sector erase */
#define OPCODE_RES			0xAB	/* Read Electronic Signature */
#define OPCODE_RDID			0x9F	/* Read JEDEC ID */

#define OPCODE_FAST_READ		0x0B	/* Fast Read */
#define OPCODE_DOR			0x3B	/* Dual Output Read */
#define OPCODE_QOR			0x6B	/* Quad Output Read */
#define OPCODE_DIOR			0xBB	/* Dual IO High Performance Read */
#define OPCODE_QIOR			0xEB	/* Quad IO High Performance Read */
#define OPCODE_READ_ID			0x90	/* Read Manufacturer and Device ID */

#define OPCODE_P4E			0x20	/* 4KB Parameter Sectore Erase */
#define OPCODE_P8E			0x40	/* 8KB Parameter Sectore Erase */
#define OPCODE_BE			0x60	/* Bulk Erase */
#define OPCODE_BE1			0xC7	/* Bulk Erase */
#define OPCODE_QPP			0x32	/* Quad Page Programing */

#define OPCODE_CLSR			0x30
#define OPCODE_RCR			0x35	/* Read Configuration Register */

#define OPCODE_BRRD			0x16
#define OPCODE_BRWR			0x17

/* Status Register bits. */
#define SR_WIP				1	/* Write in progress */
#define SR_WEL				2	/* Write enable latch */
#define SR_BP0				4	/* Block protect 0 */
#define SR_BP1				8	/* Block protect 1 */
#define SR_BP2				0x10	/* Block protect 2 */
#define SR_EPE				0x20	/* Erase/Program error */
#define SR_SRWD				0x80	/* SR write protect */

#define snor_dbg(args...)
/* #define snor_dbg(args...) do { if (1) printf(args); } while(0) */

#define udelay(x)			usleep(x)

struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};

struct chip_info *spi_chip_info;

static int snor_wait_ready(int sleep_ms);
static int snor_read_sr(u8 *val);
static int snor_write_sr(u8 *val);
static int s95_read_sr(u8 *val);
static int s95_write_sr(u8 *val);
int s95_wait_ready(int sleep_ms);
int s95_unprotect(void);

extern unsigned int bsize;

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline void snor_write_enable(void)
{
	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_WREN);
	SPI_CONTROLLER_Chip_Select_High();
}

static inline void snor_write_disable(void)
{
	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_WRDI);
	SPI_CONTROLLER_Chip_Select_High();
}
static inline void s95_write_enable(void)
{
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x06);
    SPI_CONTROLLER_Chip_Select_High();
}

static inline void s95_write_disable(void)
{
    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x04);
    SPI_CONTROLLER_Chip_Select_High();
}
/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static inline int snor_unprotect(void)
{
	u8 sr = 0;

	if (snor_read_sr(&sr) < 0) {
		printf("%s: read_sr fail: %x\n", __func__, sr);
		return -1;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
		sr = 0;
		snor_write_sr(&sr);
	}
	return 0;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int snor_wait_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms + 1) * 1000); count++) {
		if ((snor_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & (SR_WIP | SR_EPE | SR_WEL))) {
			return 0;
		}
		udelay(500);
		/* REVISIT sometimes sleeping would be best */
	}
	printf("%s: read_sr fail: %x\n", __func__, sr);
	return -1;
}

/*
 * read status register
 */
static int snor_read_rg(u8 code, u8 *val)
{
	int retval;

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(code);
	retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}

	return 0;
}

/*
 * write status register
 */
static int snor_write_rg(u8 code, u8 *val)
{
	int retval;

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(code);
	retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}

	return 0;
}

static int snor_4byte_mode(int enable)
{
	int retval;

	if (snor_wait_ready(1))
		return -1;

	if (spi_chip_info->id == 0x1) /* Spansion */
	{
		u8 br_cfn;
		u8 br = enable ? 0x81 : 0;

		snor_write_rg(OPCODE_BRWR, &br);
		snor_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br) {
			printf("4B mode switch failed %s, 0x%02x, 0x%02x\n", enable ? "enable" : "disable" , br_cfn, br);
			return -1;
		}
	} else {
		u8 code = enable ? 0xb7 : 0xe9; /* B7: enter 4B, E9: exit 4B */

		SPI_CONTROLLER_Chip_Select_Low();
		retval = SPI_CONTROLLER_Write_One_Byte(code);
		SPI_CONTROLLER_Chip_Select_High();
		if (retval) {
			printf("%s: ret: %x\n", __func__, retval);
			return -1;
		}
		if ((!enable) && (spi_chip_info->id == 0xef)) /* Winbond */
		{
			code = 0;
			snor_write_enable();
			snor_write_rg(0xc5, &code);
		}
	}
	return 0;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int snor_erase_sector(unsigned long offset)
{
	snor_dbg("%s: offset:%x\n", __func__, offset);

	/* Wait until finished previous write command. */
	if (snor_wait_ready(950))
		return -1;

	if (spi_chip_info->addr4b) {
		snor_4byte_mode(1);
	}

	/* Send write enable, then erase commands. */
	snor_write_enable();

	SPI_CONTROLLER_Chip_Select_Low();

	SPI_CONTROLLER_Write_One_Byte(OPCODE_SE);
	if (spi_chip_info->addr4b)
		SPI_CONTROLLER_Write_One_Byte((offset >> 24) & 0xff);
	SPI_CONTROLLER_Write_One_Byte((offset >> 16) & 0xff);
	SPI_CONTROLLER_Write_One_Byte((offset >> 8) & 0xff);
	SPI_CONTROLLER_Write_One_Byte(offset & 0xff);

	SPI_CONTROLLER_Chip_Select_High();

	snor_wait_ready(950);

	if (spi_chip_info->addr4b)
		snor_4byte_mode(0);

	return 0;
}

static int full_erase_chip(void)
{
	timer_start();
	/* Wait until finished previous write command. */
	if (snor_wait_ready(3))
		return -1;

	/* Send write enable, then erase commands. */
	snor_write_enable();
	snor_unprotect();

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_BE1);
	SPI_CONTROLLER_Chip_Select_High();

	snor_wait_ready(950);
	snor_write_disable();
	timer_end();

	return 0;
}

//static struct chip_info chips_data [] = {
	/* REVISIT: fill in JEDEC ids, for parts that have them */
/*	{ "AT25DF321",		0x1f, 0x47000000, 64 * 1024, 64,  0 },
	{ "AT26DF161",		0x1f, 0x46000000, 64 * 1024, 32,  0 },

	{ "F25L016",		0x8c, 0x21150000, 64 * 1024, 32,  0 }, //ESMT
	{ "F25L16QA",		0x8c, 0x41158c41, 64 * 1024, 32,  0 },
	{ "F25L032",		0x8c, 0x21160000, 64 * 1024, 64,  0 },
	{ "F25L32QA",		0x8c, 0x41168c41, 64 * 1024, 64,  0 },
	{ "F25L064",		0x8c, 0x21170000, 64 * 1024, 128, 0 },
	{ "F25L64QA",		0x8c, 0x41170000, 64 * 1024, 128, 0 },

	{ "GD25Q16",		0xc8, 0x40150000, 64 * 1024, 32,  0 },
	{ "GD25Q32",		0xc8, 0x40160000, 64 * 1024, 64,  0 },
	{ "GD25Q64CSIG",	0xc8, 0x4017c840, 64 * 1024, 128, 0 },
	{ "GD25Q128CSIG",	0xc8, 0x4018c840, 64 * 1024, 256, 0 },
	{ "YC25Q128",		0xd8, 0x4018d840, 64 * 1024, 256, 0 },
	{ "GD25Q256CSIG",	0xc8, 0x4019c840, 64 * 1024, 512, 1 },

	{ "MX25L8005M",		0xc2, 0x2014c220, 64 * 1024, 16,  0 },
	{ "MX25L1605D",		0xc2, 0x2015c220, 64 * 1024, 32,  0 },
	{ "MX25L3205D",		0xc2, 0x2016c220, 64 * 1024, 64,  0 },
	{ "MX25L6405D",		0xc2, 0x2017c220, 64 * 1024, 128, 0 },
	{ "MX25L12805D",	0xc2, 0x2018c220, 64 * 1024, 256, 0 },
	{ "MX25L25635E",	0xc2, 0x2019c220, 64 * 1024, 512, 1 },
	{ "MX25L51245G",	0xc2, 0x201ac220, 64 * 1024, 1024, 1 },

	{ "FL016AIF",		0x01, 0x02140000, 64 * 1024, 32,  0 },
	{ "FL064AIF",		0x01, 0x02160000, 64 * 1024, 128, 0 },
	{ "S25FL016P",		0x01, 0x02144D00, 64 * 1024, 32,  0 },
	{ "S25FL032P",		0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",		0x01, 0x02164D00, 64 * 1024, 128, 0 },
	{ "S25FL128P",		0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",		0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL256S",		0x01, 0x02194D01, 64 * 1024, 512, 1 },
	{ "S25FL116K",		0x01, 0x40150140, 64 * 1024, 32,  0 },
	{ "S25FL132K",		0x01, 0x40160140, 64 * 1024, 64,  0 },
	{ "S25FL164K",		0x01, 0x40170140, 64 * 1024, 128, 0 },

	{ "EN25F16",		0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25Q16",		0x1c, 0x30151c30, 64 * 1024, 32,  0 },
	{ "EN25QH16",		0x1c, 0x70151c70, 64 * 1024, 32,  0 },
	{ "EN25Q32B",		0x1c, 0x30161c30, 64 * 1024, 64,  0 },
	{ "EN25F32",		0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "EN25F64",		0x1c, 0x20171c20, 64 * 1024, 128, 0 },
	{ "EN25Q64",		0x1c, 0x30171c30, 64 * 1024, 128, 0 },
	{ "EN25QA64A",		0x1c, 0x60170000, 64 * 1024, 128, 0 },
	{ "EN25QH64A",		0x1c, 0x70171c70, 64 * 1024, 128, 0 },
	{ "EN25Q128",		0x1c, 0x30181c30, 64 * 1024, 256, 0 },
	{ "EN25Q256",		0x1c, 0x70191c70, 64 * 1024, 512, 1 },
	{ "EN25QA128A",		0x1c, 0x60180000, 64 * 1024, 256, 0 },
	{ "EN25QH128A",		0x1c, 0x70181c70, 64 * 1024, 256, 0 },

	{ "W25X05",		0xef, 0x30100000, 64 * 1024, 1,   0 },
	{ "W25X10",		0xef, 0x30110000, 64 * 1024, 2,   0 },
	{ "W25X20",		0xef, 0x30120000, 64 * 1024, 4,   0 },
	{ "W25X40",		0xef, 0x30130000, 64 * 1024, 8,   0 },
	{ "W25X80",		0xef, 0x30140000, 64 * 1024, 16,  0 },
	{ "W25X16",		0xef, 0x30150000, 64 * 1024, 32,  0 },
	{ "W25X32VS",		0xef, 0x30160000, 64 * 1024, 64,  0 },
	{ "W25X64",		0xef, 0x30170000, 64 * 1024, 128, 0 },
	{ "W25Q20CL",		0xef, 0x40120000, 64 * 1024, 4,   0 },
	{ "W25Q20BW",		0xef, 0x50120000, 64 * 1024, 4,   0 },
	{ "W25Q20EW",		0xef, 0x60120000, 64 * 1024, 4,   0 },
	{ "W25Q80",		0xef, 0x50140000, 64 * 1024, 16,  0 },
	{ "W25Q80BL",		0xef, 0x40140000, 64 * 1024, 16,  0 },
	{ "W25Q16JQ",		0xef, 0x40150000, 64 * 1024, 32,  0 },
	{ "W25Q16JM",		0xef, 0x70150000, 64 * 1024, 32,  0 },
	{ "W25Q32BV",		0xef, 0x40160000, 64 * 1024, 64,  0 },
	{ "W25Q32DW",		0xef, 0x60160000, 64 * 1024, 64,  0 },
	{ "W25Q64BV",		0xef, 0x40170000, 64 * 1024, 128, 0 },
	{ "W25Q64DW",		0xef, 0x60170000, 64 * 1024, 128, 0 },
	{ "W25Q128BV",		0xef, 0x40180000, 64 * 1024, 256, 0 },
	{ "W25Q128FW",		0xef, 0x60180000, 64 * 1024, 256, 0 },
	{ "W25Q256FV",		0xef, 0x40190000, 64 * 1024, 512, 1 },
	{ "W25Q512JV",		0xef, 0x71190000, 64 * 1024, 1024, 1 },

	{ "M25P016",		0x20, 0x20150000, 64 * 1024, 32,  0 },
	{ "N25Q032A",		0x20, 0xba161000, 64 * 1024, 64,  0 },
	{ "N25Q064A",		0x20, 0xba171000, 64 * 1024, 128, 0 },
	{ "M25P128",		0x20, 0x20180000, 64 * 1024, 256, 0 },
	{ "N25Q128A",		0x20, 0xba181000, 64 * 1024, 256, 0 },
	{ "XM25QH32B",		0x20, 0x40160000, 64 * 1024, 64,  0 },
	{ "XM25QH32A",		0x20, 0x70160000, 64 * 1024, 64,  0 },
	{ "XM25QH64A",		0x20, 0x70170000, 64 * 1024, 128, 0 },
	{ "XM25QH128A",		0x20, 0x70182070, 64 * 1024, 256, 0 },
	{ "N25Q256A",		0x20, 0xba191000, 64 * 1024, 512, 1 },
	{ "MT25QL512AB",	0x20, 0xba201044, 64 * 1024, 1024, 1 },

	{ "ZB25VQ16",		0x5e, 0x40150000, 64 * 1024, 32,  0 },
	{ "ZB25VQ32",		0x5e, 0x40160000, 64 * 1024, 64,  0 },
	{ "ZB25VQ64",		0x5e, 0x40170000, 64 * 1024, 128, 0 },
	{ "ZB25VQ128",		0x5e, 0x40180000, 64 * 1024, 256, 0 },

	{ "BY25Q16BS",		0x68, 0x40150000, 64 * 1024, 32,  0 },
	{ "BY25Q32BS",		0x68, 0x40160000, 64 * 1024, 64,  0 },
	{ "BY25Q64AS",		0x68, 0x40170000, 64 * 1024, 128, 0 },
	{ "BY25Q128AS",		0x68, 0x40180000, 64 * 1024, 256, 0 },

    { "Xint snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b)T25F32B",		0x0b, 0x40150000, 64 * 1024, 32,  0 },
	{ "XT25F32B",		0x0b, 0x40160000, 64 * 1024, 64,  0 },
	{ "XT25F64B",		0x0b, 0x40170000, 64 * 1024, 128, 0 },
	{ "XT25F128B",		0x0b, 0x40180000, 64 * 1024, 256, 0 },
        { "XT25Q128D",		0x0b, 0x60180000, 64 * 1024, 256, 0 },
	
	{ "PM25LQ016",		0x7f, 0x9d450000, 64 * 1024, 32,  0 },
	{ "PM25LQ032",		0x7f, 0x9d460000, 64 * 1024, 64,  0 },
	{ "PM25LQ064",		0x7f, 0x9d470000, 64 * 1024, 128, 0 },
	{ "PM25LQ128",		0x7f, 0x9d480000, 64 * 1024, 256, 0 },

	{ "IC25LP016",		0x9d, 0x60150000, 64 * 1024, 32,  0 },
	{ "IC25LP032",		0x9d, 0x60160000, 64 * 1024, 64,  0 },
	{ "IC25LP064",		0x9d, 0x60170000, 64 * 1024, 128, 0 },
	{ "IC25LP128",		0x9d, 0x60180000, 64 * 1024, 256, 0 },

	{ "FS25Q016",		0xa1, 0x40150000, 64 * 1024, 32,  0 },
	{ "FS25Q032",		0xa1, 0x40160000, 64 * 1024, 64,  0 },
	{ "FS25Q064",		0xa1, 0x40170000, 64 * 1024, 128, 0 },
	{ "FS25Q128",		0xa1, 0x40180000, 64 * 1024, 256, 0 },
	{ "FM25W16",		0xa1, 0x28150000, 64 * 1024, 32,  0 },
	{ "FM25W32",		0xa1, 0x28160000, 64 * 1024, 64,  0 },
	{ "FM25W64",		0xa1, 0x28170000, 64 * 1024, 128, 0 },
	{ "FM25W128",		0xa1, 0x28180000, 64 * 1024, 256, 0 },

	{ "FM25Q16A",		0xf8, 0x32150000, 64 * 1024, 32,  0 },
	{ "FM25Q32A",		0xf8, 0x32160000, 64 * 1024, 64,  0 },
	{ "FM25Q64A",		0xf8, 0x32170000, 64 * 1024, 128, 0 },
	{ "FM25Q128A",		0xf8, 0x32180000, 64 * 1024, 256, 0 },

	{ "PN25F16",		0xe0, 0x40150000, 64 * 1024, 32,  0 },
	{ "PN25F32",		0xe0, 0x40160000, 64 * 1024, 64,  0 },
	{ "PN25F64",		0xe0, 0x40170000, 64 * 1024, 128, 0 },
	{ "PN25F128",		0xe0, 0x40180000, 64 * 1024, 256, 0 },

	{ "P25Q16H",		0x85, 0x60150000, 64 * 1024, 32,  0 },
	{ "P25Q32H",		0x85, 0x60160000, 64 * 1024, 64,  0 },
	{ "P25Q64H",		0x85, 0x60170000, 64 * 1024, 128, 0 },
	{ "P25Q128H",		0x85, 0x60180000, 64 * 1024, 256, 0 },
};
*/
/*
 * read SPI flash device ID
 */
int snor_read_devid(u8 *rxbuf, int n_rx)
{
	int retval = 0;

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_RDID);

	retval = SPI_CONTROLLER_Read_NByte(rxbuf, n_rx, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}

	return 0;
}

/*
 * read status register
 */
static int snor_read_sr(u8 *val)
{
	int retval = 0;

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_RDSR);

	retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}

	return 0;
}

/*
 * write status register
 */
static int snor_write_sr(u8 *val)
{
	int retval = 0;

	SPI_CONTROLLER_Chip_Select_Low();
	SPI_CONTROLLER_Write_One_Byte(OPCODE_WRSR);

	retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
	SPI_CONTROLLER_Chip_Select_High();
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return 0;
}

/*struct chip_info *chip_prob(void)
{
	struct chip_info *info = NULL, *match = NULL;
	u8 buf[5];
	u32 jedec, jedec_strip, weight;
	int i;

	snor_read_devid(buf, 5);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] <<8) | (u32)buf[4]);
	jedec_strip = jedec & 0xffff0000;

	printf("spi device id: %x %x %x %x %x (%x)\n", buf[0], buf[1], buf[2], buf[3], buf[4], jedec);

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < sizeof(chips_data)/sizeof(chips_data[0]); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if ((info->jedec_id == jedec) || ((info->jedec_id & 0xffff0000) == jedec_strip)) {
				printf("Detected SPI NOR Flash:\e[93m %s\e[0m, Flash Size:\e[93m %ld \e[0mMB\n", info->name, (info->sector_size * info->n_sectors) >> 20);
				return info;
			}

			if (weight > (info->jedec_id ^ jedec)) {
				weight = info->jedec_id ^ jedec;
				match = info;
			}
		}
	}
	printf("SPI NOR Flash Not Detected!\n");
*/

//match = NULL; /* Not support JEDEC calculate info */

//	return match;
//}

/*long snor_init(void)
{
	spi_chip_info = chip_prob();

	if(spi_chip_info == NULL)
		return -1;

	bsize = spi_chip_info->sector_size;

	return spi_chip_info->sector_size * spi_chip_info->n_sectors;
}
*/
/*int snor_erase(unsigned long offs, unsigned long len)
{
	unsigned long plen = len;
	snor_dbg("%s: offs:%x len:%x\n", __func__, offs, len);
*/
	/* sanity checks */
/*	if (len == 0)
		return -1;

	if(!offs && len == (spi_chip_info->sector_size * spi_chip_info->n_sectors))
	{
		printf("Please Wait......\n");
		return full_erase_chip();
	}

	timer_start();

	snor_unprotect();
*/
	/* now erase those sectors */
/*	while (len > 0) {
		if (snor_erase_sector(offs)) {
			return -1;
		}

		offs += spi_chip_info->sector_size;
		len -= spi_chip_info->sector_size;
		if( timer_progress() )
		{
			printf("\bErase %ld%% [%lu] of [%lu] bytes      ", 100 * (plen - len) / plen, plen - len, plen);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}
	printf("Erase 100%% [%lu] of [%lu] bytes      \n", plen - len, plen);
	timer_end();

	return 0;
}
*/
int snor_erase_param(unsigned long offs, unsigned long len, unsigned int sector_size, unsigned int n_sectors)
{
    unsigned long plen = len;
    snor_dbg("%s: offs:%x len:%x\n", __func__, offs, len);

    /* sanity checks */
    if (len == 0)
        return -1;

    if(!offs && len == (sector_size * n_sectors))
    {
        printf("Please Wait......\n");
        return full_erase_chip();
    }

    timer_start();

    snor_unprotect();

    /* now erase those sectors */
    while (len > 0) {
        if (snor_erase_sector(offs)) {
            return -1;
        }

        offs += sector_size;
        len -= sector_size;
        if( timer_progress() )
        {
            printf("\bErase %ld%% [%lu] of [%lu] bytes      ", 100 * (plen - len) / plen, plen - len, plen);
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            fflush(stdout);
        }
    }
    printf("Erase 100%% [%lu] of [%lu] bytes      \n", plen - len, plen);
    timer_end();

    return 0;
}


/*int snor_read(unsigned char *buf, unsigned long from, unsigned long len)
{
	u32 read_addr, physical_read_addr, remain_len, data_offset;

	snor_dbg("%s: from:%x len:%x \n", __func__, from, len);
*/
	/* sanity checks */
/*	if (len == 0)
		return 0;

	timer_start();
*/
    /* Wait till previous write/erase is done. */
//	if (snor_wait_ready(1)) {
        /* REVISIT status return?? */
/*		return -1;
	}

	read_addr = from;
	remain_len = len;

	while(remain_len > 0) {

		physical_read_addr = read_addr;
		data_offset = (physical_read_addr % (spi_chip_info->sector_size));

		if (spi_chip_info->addr4b)
			snor_4byte_mode(1);

		SPI_CONTROLLER_Chip_Select_Low();
*/
		/* Set up the write data buffer. */
/*		SPI_CONTROLLER_Write_One_Byte(OPCODE_READ);

		if (spi_chip_info->addr4b)
			SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 24) & 0xff);
		SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff);
		SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff);
		SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff);

		if( (data_offset + remain_len) < spi_chip_info->sector_size )
		{
			if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], remain_len, SPI_CONTROLLER_SPEED_SINGLE)) {
				SPI_CONTROLLER_Chip_Select_High();
				if (spi_chip_info->addr4b)
					snor_4byte_mode(0);
				len = -1;
				break;
			}
			remain_len = 0;
		} else {
			if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], spi_chip_info->sector_size - data_offset, SPI_CONTROLLER_SPEED_SINGLE)) {
				SPI_CONTROLLER_Chip_Select_High();
				if (spi_chip_info->addr4b)
					snor_4byte_mode(0);
				len = -1;
				break;
			}
			remain_len -= spi_chip_info->sector_size - data_offset;
			read_addr += spi_chip_info->sector_size - data_offset;
			if( timer_progress() ) {
				printf("\bRead %ld%% [%lu] of [%lu] bytes      ", 100 * (len - remain_len) / len, len - remain_len, len);
				printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				fflush(stdout);
			}
		}

		SPI_CONTROLLER_Chip_Select_High();

        if (spi_chip_info->addr4b)
			snor_4byte_mode(0);
	}
    //printf("Read 100%% [%lu] of [%lu] bytes      \n", len - remain_len, len);
	timer_end();

	return len;
}
*/
int snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b)
{
    u32 read_addr, physical_read_addr, remain_len, data_offset;

    snor_dbg("%s: from:%x len:%x \n", __func__, from, len);

    /* sanity checks */
    if (len == 0)
        return 0;

    timer_start();
    /* Wait till previous write/erase is done. */
    if (snor_wait_ready(1)) {
        /* REVISIT status return?? */
        return -1;
    }

    read_addr = from;
    remain_len = len;

    while(remain_len > 0) {

        physical_read_addr = read_addr;
        data_offset = (physical_read_addr % (sector_size));

        if (addr4b)
            snor_4byte_mode(1);

        SPI_CONTROLLER_Chip_Select_Low();

        /* Set up the write data buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_READ);

        if (addr4b)
            SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 24) & 0xff);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff);

        if( (data_offset + remain_len) < sector_size )
        {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], remain_len, SPI_CONTROLLER_SPEED_SINGLE)) {
                SPI_CONTROLLER_Chip_Select_High();
                if (addr4b)
                    snor_4byte_mode(0);
                len = -1;
                break;
            }
            remain_len = 0;
        } else {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], sector_size - data_offset, SPI_CONTROLLER_SPEED_SINGLE)) {
                SPI_CONTROLLER_Chip_Select_High();
                if (addr4b)
                    snor_4byte_mode(0);
                len = -1;
                break;
            }
            remain_len -= sector_size - data_offset;
            read_addr += sector_size - data_offset;
            if( timer_progress() ) {
                //printf("\bRead %ld%% [%lu] of [%lu] bytes      ", 100 * (len - remain_len) / len, len - remain_len, len);
                //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                fflush(stdout);
            }
        }

        SPI_CONTROLLER_Chip_Select_High();

        if (addr4b)
            snor_4byte_mode(0);
    }
    //printf("Read 100%% [%lu] of [%lu] bytes      \n", len - remain_len, len);
    timer_end();

    return len;
}

int snor_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned int addr4b)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;
    unsigned long plen = len;

    snor_dbg("%s: to:%x len:%x \n", __func__, to, len);

    /* sanity checks */
    if (len == 0)
        return 0;

    if (to + len > len * sector_size )
        return -1;

    timer_start();
    /* Wait until finished previous write command. */
    if (snor_wait_ready(2)) {
        return -1;
    }


    /* what page do we start with? */
    page_offset = to % FLASH_PAGESIZE;

    if (addr4b)
        snor_4byte_mode(1);

    /* write everything in PAGESIZE chunks */
    while (len > 0) {
        page_size = min(len, FLASH_PAGESIZE - page_offset);
        page_offset = 0;
        /* write the next page to flash */

        snor_wait_ready(3);
        snor_write_enable();
        snor_unprotect();

        SPI_CONTROLLER_Chip_Select_Low();
        /* Set up the opcode in the write buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_PP);

        if (addr4b)
            SPI_CONTROLLER_Write_One_Byte((to >> 24) & 0xff);
        SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff);
        SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(to & 0xff);

        if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE))
            rc = page_size;
        else
            rc = 1;

        SPI_CONTROLLER_Chip_Select_High();

        snor_dbg("%s: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);

        if( timer_progress() ) {
            //printf("\bWritten %ld%% [%lu] of [%lu] bytes      ", 100 * (plen - len) / plen, plen - len, plen);
            //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            fflush(stdout);
        }

        if (rc > 0) {
            retlen += rc;
            if (rc < page_size) {
                printf("%s: rc:%x page_size:%x\n",
                        __func__, rc, page_size);
                snor_write_disable();
                return retlen - rc;
            }
        }

        len -= page_size;
        to += page_size;
        buf += page_size;
    }

    if (addr4b)
        snor_4byte_mode(0);

    snor_write_disable();

    //printf("Written 100%% [%ld] of [%ld] bytes      \n", plen - len, plen);
    timer_end();

    return retlen;
}

int s95_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{
    u32 physical_read_addr, remain_len, data_offset;
    u32 read_addr;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    snor_dbg("%s: from:%x len:%x \n", __func__, from, len);

    /* sanity checks */
    if (len == 0)
        return 0;

    timer_start();
    /* Wait till previous write/erase is done. */
    if (s95_wait_ready(1)) {
        /* REVISIT status return?? */
        return -1;
    }

    read_addr = (u32)from;
    remain_len = (u32)len;

    while(remain_len > 0) {

        physical_read_addr = read_addr;
        data_offset = (physical_read_addr % (sector_size));


        SPI_CONTROLLER_Chip_Select_Low();

        /* Set up the write data buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_READ);

        if (algorythm == 2) SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff);
        if (algorythm > 0)  SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff);

        if( (data_offset + remain_len) < sector_size )
        {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], remain_len, SPI_CONTROLLER_SPEED_SINGLE)) {
                SPI_CONTROLLER_Chip_Select_High();
                len = -1;
                break;
            }
            remain_len = 0;
        } else {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], sector_size - data_offset, SPI_CONTROLLER_SPEED_SINGLE)) {
                SPI_CONTROLLER_Chip_Select_High();
                len = -1;
                break;
            }
            remain_len -= sector_size - data_offset;
            read_addr += sector_size - data_offset;
            if( timer_progress() ) {
                //printf("\bRead %ld%% [%lu] of [%lu] bytes      ", 100 * (len - remain_len) / len, len - remain_len, len);
                //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                fflush(stdout);
            }
        }

        SPI_CONTROLLER_Chip_Select_High();

    }
    //printf("Read 100%% [%lu] of [%lu] bytes from [%u]      \n", len - remain_len, len, from);
    timer_end();

    return len;
}

int s95_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;
    unsigned long plen = len;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    snor_dbg("%s: to:%x len:%x \n", __func__, to, len);

    /* sanity checks */
    if (len == 0) return 0;

//    if (to + len > len * sector_size ){
//        printf("STRING969");
//        return -3;
//    }
    timer_start();
    /* Wait until finished previous write command. */
    if (s95_wait_ready(2)) return -1;

    /* what page do we start with? */
    page_offset = to % sector_size;//FLASH_PAGESIZE;

    /* write everything in PAGESIZE chunks */
    while (len > 0) {
        page_size = min(len, sector_size - page_offset);
        page_offset = 0;
        /* write the next page to flash */

        s95_wait_ready(3);
        s95_write_enable();
        s95_unprotect();

        SPI_CONTROLLER_Chip_Select_Low();
        /* Set up the opcode in the write buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_PP);

        if (algorythm == 2) SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff);
        if (algorythm > 0) SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(to & 0xff);

        if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE))
            rc = page_size;
        else
            rc = 1;

        SPI_CONTROLLER_Chip_Select_High();

        snor_dbg("%s: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);

        if( timer_progress() ) {
            //printf("\bWritten %ld%% [%lu] of [%lu] bytes      ", 100 * (plen - len) / plen, plen - len, plen);
            //printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            fflush(stdout);
        }

        if (rc > 0) {
            retlen += rc;
            if (rc < page_size) {
                printf("%s: rc:%x page_size:%x\n",
                        __func__, rc, page_size);
                s95_write_disable();
                return retlen - rc;
            }
        }

        len -= page_size;
        to += page_size;
        buf += page_size;
    }


    s95_write_disable();

    //printf("Written 100%% [%ld] of [%ld] bytes      \n", plen - len, plen);
    timer_end();

    return retlen;
}

static int s95_read_sr(u8 *val)
{
    int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x05);

    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
    SPI_CONTROLLER_Chip_Select_High();
    if (retval) {
        printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }

    return 0;
}
static int s95_write_sr(u8 *val)
{
    int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x01);

    retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
    SPI_CONTROLLER_Chip_Select_High();
    if (retval) {
        printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }
    return 0;
}
int s95_unprotect(void)
{
    u8 sr = 0;

    if (s95_read_sr(&sr) < 0) {
        printf("%s: read_sr fail: %x\n", __func__, sr);
        return -1;
    }

    if ((sr & (SR_BP0 | SR_BP1 )) != 0) {
        sr = 0;
        s95_write_sr(&sr);
    }
    return 0;
}

int s95_wait_ready(int sleep_ms)
{
    int count;
    int sr = 0;

    /* one chip guarantees max 5 msec wait here after page writes,
     * but potentially three seconds (!) after page erase.
     */
    for (count = 0; count < ((sleep_ms + 1) * 1000); count++) {
        //printf("sr= %x\n",snor_read_sr((u8 *)&sr));
        if ((snor_read_sr((u8 *)&sr)) < 0)
            break;
        else if (!(sr & (SR_WIP | SR_WEL))) {
            return 0;
        }
        udelay(500);
        /* REVISIT sometimes sleeping would be best */
    }
    printf("%s: read_sr fail: %x\n", __func__, sr);
    return -1;
}
/*int snor_write(unsigned char *buf, unsigned long to, unsigned long len)
{
	u32 page_offset, page_size;
	int rc = 0, retlen = 0;
	unsigned long plen = len;

	snor_dbg("%s: to:%x len:%x \n", __func__, to, len);
*/
	/* sanity checks */
/*	if (len == 0)
		return 0;

	if (to + len > spi_chip_info->sector_size * spi_chip_info->n_sectors)
		return -1;

	timer_start();
*/
	/* Wait until finished previous write command. */
/*	if (snor_wait_ready(2)) {
		return -1;
	}

*/
	/* what page do we start with? */
/*	page_offset = to % FLASH_PAGESIZE;

	if (spi_chip_info->addr4b)
		snor_4byte_mode(1);
*/
	/* write everything in PAGESIZE chunks */
/*	while (len > 0) {
		page_size = min(len, FLASH_PAGESIZE - page_offset);
		page_offset = 0;
*/
        /* write the next page to flash */
/*
		snor_wait_ready(3);
		snor_write_enable();
		snor_unprotect();

		SPI_CONTROLLER_Chip_Select_Low();
*/
    /* Set up the opcode in the write buffer. */
/*		SPI_CONTROLLER_Write_One_Byte(OPCODE_PP);

		if (spi_chip_info->addr4b)
			SPI_CONTROLLER_Write_One_Byte((to >> 24) & 0xff);
		SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff);
		SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff);
		SPI_CONTROLLER_Write_One_Byte(to & 0xff);

		if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE))
			rc = page_size;
		else
			rc = 1;

		SPI_CONTROLLER_Chip_Select_High();

		snor_dbg("%s: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);

		if( timer_progress() ) {
			printf("\bWritten %ld%% [%lu] of [%lu] bytes      ", 100 * (plen - len) / plen, plen - len, plen);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}

		if (rc > 0) {
			retlen += rc;
			if (rc < page_size) {
				printf("%s: rc:%x page_size:%x\n",
						__func__, rc, page_size);
				snor_write_disable();
				return retlen - rc;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
	}

	if (spi_chip_info->addr4b)
		snor_4byte_mode(0);

	snor_write_disable();

	printf("Written 100%% [%ld] of [%ld] bytes      \n", plen - len, plen);
	timer_end();

	return retlen;
}
*/
/*void support_snor_list(void)
{
	int i;

	printf("SPI NOR Flash Support List:\n");
	for ( i = 0; i < (sizeof(chips_data)/sizeof(struct chip_info)); i++)
	{
		printf("%03d. %s\n", i + 1, chips_data[i].name);
	}
}
*/
/* End of [spi_nor_flash.c] package */
