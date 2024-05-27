/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * 2023-2024 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
/*
struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};
*/
unsigned char algType = 0;

//struct chip_info *spi_chip_info;

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

    //if (spi_chip_info->id == 0x1) /* Spansion */
    if (algType == 0x02) /* Spansion */
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
        //if ((!enable) && (spi_chip_info->id == 0xef)) /* Winbond */
        if ((!enable) && (algType == 0x01)) /* Winbond */
		{
			code = 0;
			snor_write_enable();
			snor_write_rg(0xc5, &code);
		}
	}
	return 0;
}


int full_erase_chip(void)
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


int snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b)
{
    u32 read_addr, physical_read_addr, remain_len, data_offset;

    //addr4bit transforming
    algType = (addr4b & 0xf0) >> 4;
    addr4b = addr4b & 0x0f;

    snor_dbg("%s: from:%x len:%x \n", __func__, from, len);

    /* sanity checks */
    if (len == 0)
        return 0;

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

        }

        SPI_CONTROLLER_Chip_Select_High();

        if (addr4b)
            snor_4byte_mode(0);
    }

    return len;
}

int snor_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned int addr4b)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;

    //addr4bit transforming
    algType = (addr4b & 0xf0) >> 4;
    addr4b = addr4b & 0x0f;

    snor_dbg("%s: to:%x len:%x \n", __func__, to, len);

    /* sanity checks */
    if (len == 0)
        return 0;

    if (to + len > len * sector_size )
        return -1;

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

    return retlen;
}

int s95_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{
    u32 physical_read_addr, remain_len, data_offset;
    u32 read_addr;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    unsigned char a8 = currentAlgorithm & 0x10;
    snor_dbg("%s: from:%x len:%x \n", __func__, from, len);

    /* sanity checks */
    if (len == 0)
        return 0;

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
        if ((from > 255) && (a8 > 0)) SPI_CONTROLLER_Write_One_Byte(0x0b); //read command + a8 bit
        else SPI_CONTROLLER_Write_One_Byte(0x03); //read command

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
        }

        SPI_CONTROLLER_Chip_Select_High();

    }

    return len;
}

int s95_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;
    unsigned long plen = len;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    unsigned char a8 = currentAlgorithm & 0x10;
    snor_dbg("%s: to:%x len:%x \n", __func__, to, len);

    /* sanity checks */
    if (len == 0) return 0;

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
        if ((to > 255) && (a8 > 0)) SPI_CONTROLLER_Write_One_Byte(0x0a); //write command + a8 bit
        else SPI_CONTROLLER_Write_One_Byte(0x02); //write command

        if (algorythm == 2) SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff);
        if (algorythm > 0) SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(to & 0xff);

        if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE))
            rc = page_size;
        else
            rc = 1;

        SPI_CONTROLLER_Chip_Select_High();

        snor_dbg("%s: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);

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


    return retlen;
}


int s95_full_erase(void)
{
    timer_start();
    /* Wait until finished previous write command. */
    if (s95_wait_ready(3))
        return -1;

    /* Send write enable, then erase commands. */
    s95_write_enable();
    s95_unprotect();

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x62);
    SPI_CONTROLLER_Chip_Select_High();

    s95_wait_ready(950);
    s95_write_disable();
    timer_end();

    return 0;

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

int at45_read_sr(u8 *val)
{
    int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0xd7);
    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE);
    SPI_CONTROLLER_Chip_Select_High();
    if (retval) {
        //printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }

    return 0;
}

int at45_wait_ready(int sleep_ms)
{
    int count;
    int sr = 0;

    /* one chip guarantees max 5 msec wait here after page writes,
     * but potentially three seconds (!) after page erase.
     */
    for (count = 0; count < ((sleep_ms + 1) * 1000); count++) {
        //printf("sr= %x\n",snor_read_sr((u8 *)&sr));
        if ((at45_read_sr((u8 *)&sr)) < 0)
            break;
        else if ((sr & 0x80)) {
            return 0;
        }
        udelay(500);
        /* REVISIT sometimes sleeping would be best */
    }
    printf("%s: read_sr fail: %x\n", __func__, sr);
    return -1;
}

int at45_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{
    u32 physical_read_addr, remain_len, data_offset;
    u32 read_addr;
    unsigned char algorythm = (currentAlgorithm & 0xf0) >> 8;
    unsigned char addrLen = 9;
    unsigned int sector_addr = 0;
    int retval;
    if (sector_size > 511) addrLen++;

    /* sanity checks */
    if (len == 0)
        return 0;

    /* Wait till previous write/erase is done. */
    if (at45_wait_ready(1)) {
        /* REVISIT status return?? */
        return -1;
    }

    read_addr = (u32)from;
    sector_addr = read_addr / sector_size;
    read_addr = read_addr % len;
    read_addr = read_addr + (sector_addr << addrLen);
    remain_len = (u32)len;

        physical_read_addr = read_addr;
        data_offset = (physical_read_addr % (sector_size));


        SPI_CONTROLLER_Chip_Select_Low();

        //SPI_CONTROLLER_Write_One_Byte(0x52); //read command
        SPI_CONTROLLER_Write_One_Byte(0xE8); //read command
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff);

        SPI_CONTROLLER_Write_One_Byte(0xff);
        SPI_CONTROLLER_Write_One_Byte(0xff);
        SPI_CONTROLLER_Write_One_Byte(0xff);
        SPI_CONTROLLER_Write_One_Byte(0xff);

        retval = SPI_CONTROLLER_Read_NByte(&buf[0],sector_size,SPI_CONTROLLER_SPEED_SINGLE);

        if (retval)
        {
           return 0;//error
        }
         SPI_CONTROLLER_Chip_Select_High();
     return 1;
}


int at45_write_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm)
{    u32 physical_read_addr, remain_len, data_offset;
     u32 read_addr;
     unsigned char algorythm = (currentAlgorithm & 0xf0) >> 8;
     unsigned char addrLen = 9;
     unsigned int sector_addr = 0;
     int retval;
     if (sector_size > 511) addrLen++;

     /* sanity checks */
     if (len == 0) return 0;
     /* Wait till previous write/erase is done. */
     if (at45_wait_ready(1)) {
         /* REVISIT status return?? */
         return -1;
     }

     read_addr = (u32)from;
     sector_addr = read_addr / sector_size;
     read_addr = read_addr % len;
     read_addr = read_addr + (sector_addr << addrLen);
     remain_len = (u32)len;

         physical_read_addr = read_addr;
         data_offset = (physical_read_addr % (sector_size));


         SPI_CONTROLLER_Chip_Select_Low();

         SPI_CONTROLLER_Write_One_Byte(0x82); //write command
         SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff);
         SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff);
         SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff);

         //SPI_CONTROLLER_Write_One_Byte(0xff);

         retval = SPI_CONTROLLER_Write_NByte(buf, sector_size, SPI_CONTROLLER_SPEED_SINGLE);

         if (retval)
         {
            return 0;//error
         }
          SPI_CONTROLLER_Chip_Select_High();

      return 1;
 }

int at45_full_erase(void)
{
    /* Wait until finished previous write command. */
    if (s95_wait_ready(3))
        return -1;

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0xC7);
    SPI_CONTROLLER_Write_One_Byte(0x94);
    SPI_CONTROLLER_Write_One_Byte(0x80);
    SPI_CONTROLLER_Write_One_Byte(0x9A);
    SPI_CONTROLLER_Chip_Select_High();

    at45_wait_ready(950);
    return 0;

}


int at45_sector_erase(unsigned int sectorNumber,  unsigned int pageSize)
{
    u32 send_addr = 0;
    unsigned char fullLen = 0x0c;
    if (pageSize > 511) fullLen++;
    send_addr = sectorNumber << fullLen;
    /* Wait until finished previous write command. */
    if (at45_wait_ready(1))
        return -1;

    SPI_CONTROLLER_Chip_Select_Low();
    SPI_CONTROLLER_Write_One_Byte(0x50); //block erase command
    SPI_CONTROLLER_Write_One_Byte((send_addr >> 16) & 0xff);
    SPI_CONTROLLER_Write_One_Byte((send_addr >> 8) & 0xff);
    SPI_CONTROLLER_Write_One_Byte(send_addr & 0xff);
    SPI_CONTROLLER_Chip_Select_High();

    at45_wait_ready(1);
    return 0;

}
