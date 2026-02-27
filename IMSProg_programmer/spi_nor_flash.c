/*
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 * 2023-2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
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
u8 programmerType = 0;
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
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_WREN, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
}

static inline void snor_write_disable(void)
{
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_WRDI, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
}
static inline void s95_write_enable(void)
{
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x06, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
}

static inline void s95_write_disable(void)
{
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x04, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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
int snor_wait_ready(int sleep_ms)
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

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(code, programmerType);
    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(code, programmerType);
    retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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

        SPI_CONTROLLER_Chip_Select_Low(programmerType);
        retval = SPI_CONTROLLER_Write_One_Byte(code, programmerType);
        SPI_CONTROLLER_Chip_Select_High(programmerType);
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

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_BE1, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

	snor_wait_ready(950);
	snor_write_disable();
	timer_end();

	return 0;
}

int snor_block_erase(unsigned int sector_number, unsigned int blockSize, u8 addr4b, u8 progType)
{
    unsigned int physical_addr;
    //addr4bit transforming
    algType = (addr4b & 0xf0) >> 4;
    addr4b = addr4b & 0x0f;

    programmerType = progType;

    if (addr4b) snor_4byte_mode(1);

    physical_addr = sector_number * blockSize;
    /* Wait until finished previous write command. */
    if (snor_wait_ready(950)) return -1;
    snor_write_enable();
    snor_unprotect();

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xd8, programmerType);
    if (addr4b) SPI_CONTROLLER_Write_One_Byte((physical_addr >> 24) & 0xff, programmerType);

    SPI_CONTROLLER_Write_One_Byte((physical_addr >> 16) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte((physical_addr >> 8) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte(physical_addr & 0xff, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

    snor_write_disable();
    if (addr4b) snor_4byte_mode(0);
    return 0;
}

/*
 * read SPI flash device ID
 */
int snor_read_devid(u8 *rxbuf, int n_rx, uint8_t progType)
{
	int retval = 0;
    programmerType = progType;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_RDID, programmerType);

    retval = SPI_CONTROLLER_Read_NByte(rxbuf, n_rx, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}

    if ((rxbuf[0] == 0) && (rxbuf[1] == 0) && (rxbuf[2] == 0))
    {
        rxbuf[0] = 0xff;
        rxbuf[1] = 0xff;
        rxbuf[2] = 0xff;
    }

	return 0;
}

/*
 * read status register
 */
static int snor_read_sr(u8 *val)
{
	int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_RDSR, programmerType);

    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_WRSR, programmerType);

    retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
	if (retval) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return 0;
}


int snor_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned int addr4b, uint8_t progType)
{
    u32 read_addr, physical_read_addr, remain_len, data_offset;
    programmerType = progType;
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

        SPI_CONTROLLER_Chip_Select_Low(programmerType);

        /* Set up the write data buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_READ, programmerType);

        if (addr4b)
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 24) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff, programmerType);

        if( (data_offset + remain_len) < sector_size )
        {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], remain_len, SPI_CONTROLLER_SPEED_SINGLE, programmerType)) {
                SPI_CONTROLLER_Chip_Select_High(programmerType);
                if (addr4b)
                    snor_4byte_mode(0);
                len = -1;
                break;
            }
            remain_len = 0;
        } else {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], sector_size - data_offset, SPI_CONTROLLER_SPEED_SINGLE, programmerType)) {
                SPI_CONTROLLER_Chip_Select_High(programmerType);
                if (addr4b)
                    snor_4byte_mode(0);
                len = -1;
                break;
            }
            remain_len -= sector_size - data_offset;
            read_addr += sector_size - data_offset;

        }

        SPI_CONTROLLER_Chip_Select_High(programmerType);

        if (addr4b)
            snor_4byte_mode(0);
    }

    return len;
}

int snor_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned int addr4b, u8 progType)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;

    programmerType = progType;

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

        SPI_CONTROLLER_Chip_Select_Low(programmerType);
        /* Set up the opcode in the write buffer. */
        SPI_CONTROLLER_Write_One_Byte(OPCODE_PP, programmerType);

        if (addr4b)
            SPI_CONTROLLER_Write_One_Byte((to >> 24) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(to & 0xff, programmerType);

        if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE, programmerType))
            rc = page_size;
        else
            rc = 1;

        SPI_CONTROLLER_Chip_Select_High(programmerType);

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

int s95_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm, u8 progType)
{
    u32 physical_read_addr, remain_len, data_offset;
    u32 read_addr;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    unsigned char a8 = currentAlgorithm & 0x10;
    snor_dbg("%s: from:%x len:%x \n", __func__, from, len);

    programmerType = progType;

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


        SPI_CONTROLLER_Chip_Select_Low(programmerType);

        /* Set up the write data buffer. */
        if ((from > 255) && (a8 > 0)) SPI_CONTROLLER_Write_One_Byte(0x0b, programmerType); //read command + a8 bit
        else SPI_CONTROLLER_Write_One_Byte(0x03, programmerType); //read command

        if (algorythm == 2) SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff, programmerType);
        if (algorythm > 0)  SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff, programmerType);

        if( (data_offset + remain_len) < sector_size )
        {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], remain_len, SPI_CONTROLLER_SPEED_SINGLE, programmerType)) {
                SPI_CONTROLLER_Chip_Select_High(programmerType);
                len = -1;
                break;
            }
            remain_len = 0;
        } else {
            if(SPI_CONTROLLER_Read_NByte(&buf[len - remain_len], sector_size - data_offset, SPI_CONTROLLER_SPEED_SINGLE, programmerType)) {
                SPI_CONTROLLER_Chip_Select_High(programmerType);
                len = -1;
                break;
            }
            remain_len -= sector_size - data_offset;
            read_addr += sector_size - data_offset;
        }

        SPI_CONTROLLER_Chip_Select_High(programmerType);

    }

    return len;
}

int s95_write_param(unsigned char *buf, unsigned long to, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm, u8 progType)
{
    u32 page_offset, page_size;
    int rc = 0, retlen = 0;
    unsigned long plen = len;
    unsigned char algorythm = currentAlgorithm & 0x0f;
    unsigned char a8 = currentAlgorithm & 0x10;
    snor_dbg("%s: to:%x len:%x \n", __func__, to, len);

    programmerType = progType;

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

        SPI_CONTROLLER_Chip_Select_Low(programmerType);
        /* Set up the opcode in the write buffer. */
        if ((to > 255) && (a8 > 0)) SPI_CONTROLLER_Write_One_Byte(0x0a, programmerType); //write command + a8 bit
        else SPI_CONTROLLER_Write_One_Byte(0x02, programmerType); //write command

        if (algorythm == 2) SPI_CONTROLLER_Write_One_Byte((to >> 16) & 0xff, programmerType);
        if (algorythm > 0) SPI_CONTROLLER_Write_One_Byte((to >> 8) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(to & 0xff, programmerType);

        if(!SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE, programmerType))
            rc = page_size;
        else
            rc = 1;

        SPI_CONTROLLER_Chip_Select_High(programmerType);

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


int s95_full_erase(u8 progType)
{
    programmerType = progType;

    timer_start();
    /* Wait until finished previous write command. */
    if (s95_wait_ready(3))
        return -1;

    /* Send write enable, then erase commands. */
    s95_write_enable();
    s95_unprotect();

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x62, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

    s95_wait_ready(950);
    s95_write_disable();
    timer_end();

    return 0;

}


static int s95_read_sr(u8 *val)
{
    int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x05, programmerType);
    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    if (retval) {
        printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }

    return 0;
}
static int s95_write_sr(u8 *val)
{
    int retval = 0;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x01, programmerType);

    retval = SPI_CONTROLLER_Write_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xd7, programmerType);
    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
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

int at45_read_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm, u8 progType)
{
    u32 physical_read_addr, remain_len, data_offset;
    u32 read_addr;
    unsigned char algorythm = (currentAlgorithm & 0xf0) >> 8;
    unsigned char addrLen = 9;
    unsigned int sector_addr = 0;
    int retval;

    programmerType = progType;

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


        SPI_CONTROLLER_Chip_Select_Low(programmerType);

        //SPI_CONTROLLER_Write_One_Byte(0x52); //read command
        SPI_CONTROLLER_Write_One_Byte(0xE8, programmerType); //read command
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff, programmerType);

        SPI_CONTROLLER_Write_One_Byte(0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(0xff, programmerType);
        SPI_CONTROLLER_Write_One_Byte(0xff, programmerType);

        retval = SPI_CONTROLLER_Read_NByte(&buf[0],sector_size,SPI_CONTROLLER_SPEED_SINGLE, programmerType);

        if (retval)
        {
           return 0;//error
        }
         SPI_CONTROLLER_Chip_Select_High(programmerType);
     return 1;
}


int at45_write_param(unsigned char *buf, unsigned long from, unsigned long len, unsigned int sector_size, unsigned char currentAlgorithm, u8 progType)
{    u32 physical_read_addr, remain_len, data_offset;
     u32 read_addr;
     unsigned char algorythm = (currentAlgorithm & 0xf0) >> 8;
     unsigned char addrLen = 9;
     unsigned int sector_addr = 0;
     int retval;

     programmerType = progType;

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


         SPI_CONTROLLER_Chip_Select_Low(programmerType);

         SPI_CONTROLLER_Write_One_Byte(0x82, programmerType); //write command
         SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 16) & 0xff, programmerType);
         SPI_CONTROLLER_Write_One_Byte((physical_read_addr >> 8) & 0xff, programmerType);
         SPI_CONTROLLER_Write_One_Byte(physical_read_addr & 0xff, programmerType);

         //SPI_CONTROLLER_Write_One_Byte(0xff);

         retval = SPI_CONTROLLER_Write_NByte(buf, sector_size, SPI_CONTROLLER_SPEED_SINGLE, programmerType);

         if (retval)
         {
            return 0;//error
         }
          SPI_CONTROLLER_Chip_Select_High(programmerType);

      return 1;
 }

int at45_full_erase(u8 progType)
{
    programmerType = progType;

    /* Wait until finished previous write command. */
    if (s95_wait_ready(3))
        return -1;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xC7, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x94, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x80, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x9A, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

    at45_wait_ready(950);
    return 0;

}


int at45_sector_erase(unsigned int sectorNumber,  unsigned int pageSize, u8 progType)
{
    u32 send_addr = 0;
    unsigned char fullLen = 0x0c;

    programmerType = progType;

    if (pageSize > 511) fullLen++;
    send_addr = sectorNumber << fullLen;
    /* Wait until finished previous write command. */
    if (at45_wait_ready(1))
        return -1;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x50, programmerType); //block erase command
    SPI_CONTROLLER_Write_One_Byte((send_addr >> 16) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte((send_addr >> 8) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte(send_addr & 0xff, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

    at45_wait_ready(1);
    return 0;

}

static int nand_read_main_sr(u8 *val)
{
    int retval = 0;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x0f, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xc0, programmerType); //main status / feature register address

    retval = SPI_CONTROLLER_Read_NByte(val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    if (retval) {
        printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }

    return 0;
}


int nand_wait_ready(int sleep_ms)
{
    int count;
    int sr = 0;

    /* one chip guarantees max 5 msec wait here after page writes,
     * but potentially three seconds (!) after page erase.
     */
    for (count = 0; count < ((sleep_ms + 1) * 1000); count++) {
        if ((nand_read_main_sr((u8 *)&sr)) < 0) break; //error reading status register
        if ((sr & 1) == 0) return 0; // BUSY=0
        udelay(500);
        /* REVISIT sometimes sleeping would be best */
    }
    printf("%s: read_sr fail: %x\n", __func__, sr);
    return -1;
}


int nand_page_read(unsigned char *buf, unsigned int page_size, uint32_t sector_number)
{
    int retval;
    unsigned char cmdbuf[4];
    cmdbuf[0] = 0x13;//From sector to chip buffer
    cmdbuf[1] = (sector_number >> 16) & 0xff;
    cmdbuf[2] = (sector_number >> 8) & 0xff;
    cmdbuf[3] = sector_number & 0xff;
    nand_wait_ready(100);
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_NByte( cmdbuf, 4, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    nand_wait_ready(100);
    cmdbuf[0] = 0x03;//Reading from buffer
    cmdbuf[1] = 0x00;
    cmdbuf[2] = 0x00;
    cmdbuf[3] = 0x00;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_NByte( cmdbuf, 4, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    retval = SPI_CONTROLLER_Read_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    return retval;
}

int nand_read_devid(u8 *rxbuf, int n_rx, uint8_t progType)
{
    int retval = 0;
    programmerType = progType;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_RDID, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x00, programmerType);
    retval = SPI_CONTROLLER_Read_NByte(rxbuf, n_rx, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    if (retval) {
        printf("%s: ret: %x\n", __func__, retval);
        return retval;
    }

    return 0;
}

void nand_write_enable(void)
{
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_WREN, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    usleep(1);
}

static inline void nand_write_disable(void)
{
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(OPCODE_WRDI, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    usleep(1);
}

int nand_block_erase(unsigned int sector_number, unsigned int blockSize, u8 progType)
{

    /* Wait until finished previous write command. */
    if (nand_wait_ready(950)) return -1;
    sector_number = sector_number << 6;
    // PA[15:6] is the address for 128KB blocks (total 1,024 blocks), PA[5:0] is
    //the address for 2KB pages (total 64 pages for each block)
    nand_write_enable();
    nand_unprotect(programmerType);
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xd8, programmerType);
    SPI_CONTROLLER_Write_One_Byte((sector_number >> 16) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte((sector_number >>  8) & 0xff, programmerType);
    SPI_CONTROLLER_Write_One_Byte(sector_number & 0xff, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);

    //nand_write_disable();
    return 0;
}


int nand_page_write(unsigned char *buf, unsigned int page_size, uint32_t sector_number)
{
    int retval;
    unsigned char cmdbuf[4];
    if (nand_wait_ready(950)) return -1;
    nand_write_enable();
    cmdbuf[0] = 0x02; //From PC to buffer
    cmdbuf[1] = 0x00;
    cmdbuf[2] = 0x00;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    retval = SPI_CONTROLLER_Write_NByte(cmdbuf, 3, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    if (retval == -1) return retval;
    retval = SPI_CONTROLLER_Write_NByte(buf, page_size, SPI_CONTROLLER_SPEED_SINGLE, programmerType); //1010 page_size УБРАТЬ /8 !!!!
    if (retval == -1) return retval;
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    nand_wait_ready(200);
    nand_write_enable();

    cmdbuf[0] = 0x10; //From buffer to chip
    cmdbuf[1] = (sector_number >> 16) & 0xff;
    cmdbuf[2] = (sector_number >>  8) & 0xff;
    cmdbuf[3] = sector_number & 0xff;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    retval = SPI_CONTROLLER_Write_NByte(cmdbuf, 4, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    return retval;
}

int nand_block_read(unsigned char *buf, unsigned int page_size, uint32_t block_number, unsigned int pages_per_block, uint8_t progType)
{
    int retval;
    unsigned int i;
    uint32_t secNumber;
    programmerType = progType;
    for (i = 0; i < pages_per_block; i++)
    {
        secNumber = block_number * pages_per_block + i;
        retval = nand_page_read(&buf[i * page_size], page_size, secNumber);
        if (retval == -1) return retval;
    }
    return 0;
}

int nand_block_write(unsigned char *buf, unsigned int page_size, uint32_t block_number, unsigned int pages_per_block, u8 progType)
{
    int retval;
    unsigned int i;
    uint32_t secNumber;

    programmerType = progType;

    for (i = 0; i < pages_per_block; i++)
    {
        secNumber = block_number * pages_per_block + i;
        retval = nand_page_write(&buf[i * page_size], page_size, secNumber);
        if (retval == -1) return retval;
    }
    return 0;
}

void nand_unprotect(u8 progType)
{
    programmerType = progType;

    nand_write_enable();
    u8 prot_reg;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x0f, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xa0, programmerType);
    SPI_CONTROLLER_Read_NByte(&prot_reg, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    usleep(1);
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x1f, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xa0, programmerType);
    SPI_CONTROLLER_Write_One_Byte(prot_reg & 0x83, programmerType); //set to 0 bytes 6,5,4,3,2
    SPI_CONTROLLER_Chip_Select_High(programmerType);
}

void nand_ECCEnable(u8 progType)
{
    u8 val;

    programmerType = progType;

    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x0f, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xb0, programmerType);
    SPI_CONTROLLER_Read_NByte(&val, 1, SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    usleep(2);
    nand_write_enable();
    //val = val | 0x10;
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x1f, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0xb0, programmerType);
    SPI_CONTROLLER_Write_One_Byte(val | 0x10, programmerType);  //set to 1 byte 4
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    usleep(2);
}

int nand_checkBadBlock(uint32_t blockNo, uint32_t sectSize, uint32_t blockPerSector, u8 progType)
{
    int retval; // Return: -1 - error operation, 0 - good block, 1 - bad block
    unsigned char buf[2];
    uint32_t sectNo;
    sectNo = blockNo * blockPerSector;

    programmerType = progType;

    nand_wait_ready(950);
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x13, programmerType);
    SPI_CONTROLLER_Write_One_Byte((0xff0000 & sectNo) >> 16, programmerType);
    SPI_CONTROLLER_Write_One_Byte((0x00ff00 & sectNo) >> 8, programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x0000ff & sectNo, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    //usleep(1000);
    nand_wait_ready(950);
    SPI_CONTROLLER_Chip_Select_Low(programmerType);
    SPI_CONTROLLER_Write_One_Byte(0x03, programmerType);
//    SPI_CONTROLLER_Write_One_Byte(0x08); //high address
//    SPI_CONTROLLER_Write_One_Byte(0x00); //low address
    SPI_CONTROLLER_Write_One_Byte((0x00ff00 & sectSize) >> 8, programmerType); //high address
    SPI_CONTROLLER_Write_One_Byte(0x0000ff & sectSize, programmerType); //low address
    SPI_CONTROLLER_Write_One_Byte(0x00, programmerType); //dymmy byte
    retval = SPI_CONTROLLER_Read_NByte(buf,2,SPI_CONTROLLER_SPEED_SINGLE, programmerType);
    SPI_CONTROLLER_Chip_Select_High(programmerType);
    if (retval == -1) return retval;
    if (buf[0] == 0xff) return 0;
    else return 1;
}
