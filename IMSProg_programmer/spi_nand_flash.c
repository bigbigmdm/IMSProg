/*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_nand_flash.c
 * DATE:
 * VERSION: 1.00
 * PURPOSE: To Provide SPI NAND Access interface.
 * NOTES:
 *
 * AUTHOR :
 *
 * FUNCTIONS
 *
 *      SPI_NAND_Flash_Init             To provide interface for SPI NAND init.
 *      SPI_NAND_Flash_Write_Nbyte      To provide interface for Write N Bytes into SPI NAND Flash.
 *      SPI_NAND_Flash_Read_Byte        To provide interface for read 1 Bytes from SPI NAND Flash.
 *      SPI_NAND_Flash_Read_DWord       To provide interface for read Double Word from SPI NAND Flash.
 *      SPI_NAND_Flash_Read_NByte       To provide interface for Read N Bytes from SPI NAND Flash.
 *      SPI_NAND_Flash_Erase            To provide interface for Erase SPI NAND Flash.
 * 
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 *
 *======================================================================================
 */

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "types.h"
#include "spi_nand_flash.h"
#include "spi_controller.h"
#include "nandcmd_api.h"
#include "timer.h"

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */

/* SPI NAND Command Set */
#define _SPI_NAND_OP_GET_FEATURE			0x0F	/* Get Feature */
#define _SPI_NAND_OP_SET_FEATURE			0x1F	/* Set Feature */
#define _SPI_NAND_OP_PAGE_READ				0x13	/* Load page data into cache of SPI NAND chip */
#define _SPI_NAND_OP_READ_FROM_CACHE_SINGLE		0x03	/* Read data from cache of SPI NAND chip, single speed*/
#define _SPI_NAND_OP_READ_FROM_CACHE_DUAL		0x3B	/* Read data from cache of SPI NAND chip, dual speed*/
#define _SPI_NAND_OP_READ_FROM_CACHE_QUAD		0x6B	/* Read data from cache of SPI NAND chip, quad speed*/
#define _SPI_NAND_OP_WRITE_ENABLE			0x06	/* Enable write data to  SPI NAND chip */
#define _SPI_NAND_OP_WRITE_DISABLE			0x04	/* Reseting the Write Enable Latch (WEL) */
#define _SPI_NAND_OP_PROGRAM_LOAD_SINGLE		0x02	/* Write data into cache of SPI NAND chip with cache reset, single speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_QUAD			0x32	/* Write data into cache of SPI NAND chip with cache reset, quad speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_RAMDOM_SINGLE		0x84	/* Write data into cache of SPI NAND chip, single speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_RAMDON_QUAD		0x34	/* Write data into cache of SPI NAND chip, quad speed */

#define _SPI_NAND_OP_PROGRAM_EXECUTE			0x10	/* Write data from cache into SPI NAND chip */
#define _SPI_NAND_OP_READ_ID				0x9F	/* Read Manufacture ID and Device ID */
#define _SPI_NAND_OP_BLOCK_ERASE			0xD8	/* Erase Block */
#define _SPI_NAND_OP_RESET				0xFF	/* Reset */
#define _SPI_NAND_OP_DIE_SELECT				0xC2	/* Die Select */

/* SPI NAND register address of command set */
#define _SPI_NAND_ADDR_ECC				0x90	/* Address of ECC Config */
#define _SPI_NAND_ADDR_PROTECTION			0xA0	/* Address of protection */
#define _SPI_NAND_ADDR_FEATURE				0xB0	/* Address of feature */
#define _SPI_NAND_ADDR_STATUS				0xC0	/* Address of status */
#define _SPI_NAND_ADDR_FEATURE_4			0xD0	/* Address of status 4 */
#define _SPI_NAND_ADDR_STATUS_5				0xE0	/* Address of status 5 */
#define _SPI_NAND_ADDR_MANUFACTURE_ID			0x00	/* Address of Manufacture ID */
#define _SPI_NAND_ADDR_DEVICE_ID			0x01	/* Address of Device ID */

/* SPI NAND value of register address of command set */
#define _SPI_NAND_VAL_DISABLE_PROTECTION		0x0	/* Value for disable write protection */
#define _SPI_NAND_VAL_ENABLE_PROTECTION			0x38	/* Value for enable write protection */
#define _SPI_NAND_VAL_OIP				0x1	/* OIP = Operaton In Progress */
#define _SPI_NAND_VAL_ERASE_FAIL			0x4	/* E_FAIL = Erase Fail */
#define _SPI_NAND_VAL_PROGRAM_FAIL			0x8	/* P_FAIL = Program Fail */

/* SPI NAND Size Define */
#define _SPI_NAND_PAGE_SIZE_512				0x0200
#define _SPI_NAND_PAGE_SIZE_2KBYTE			0x0800
#define _SPI_NAND_PAGE_SIZE_4KBYTE			0x1000
#define _SPI_NAND_OOB_SIZE_64BYTE			0x40
#define _SPI_NAND_OOB_SIZE_120BYTE			0x78
#define _SPI_NAND_OOB_SIZE_128BYTE			0x80
#define _SPI_NAND_OOB_SIZE_256BYTE			0x100
#define _SPI_NAND_BLOCK_SIZE_128KBYTE			0x20000
#define _SPI_NAND_BLOCK_SIZE_256KBYTE			0x40000
#define _SPI_NAND_CHIP_SIZE_512MBIT			0x04000000
#define _SPI_NAND_CHIP_SIZE_1GBIT			0x08000000
#define _SPI_NAND_CHIP_SIZE_2GBIT			0x10000000
#define _SPI_NAND_CHIP_SIZE_4GBIT			0x20000000

/* SPI NAND Manufacturers ID */
#define _SPI_NAND_MANUFACTURER_ID_GIGADEVICE		0xC8
#define _SPI_NAND_MANUFACTURER_ID_WINBOND		0xEF
#define _SPI_NAND_MANUFACTURER_ID_ESMT			0xC8
#define _SPI_NAND_MANUFACTURER_ID_MXIC			0xC2
#define _SPI_NAND_MANUFACTURER_ID_ZENTEL		0xC8
#define _SPI_NAND_MANUFACTURER_ID_ETRON			0xD5
#define _SPI_NAND_MANUFACTURER_ID_TOSHIBA		0x98
#define _SPI_NAND_MANUFACTURER_ID_MICRON		0x2C
#define _SPI_NAND_MANUFACTURER_ID_HEYANG		0xC9
#define _SPI_NAND_MANUFACTURER_ID_PN			0xA1
#define _SPI_NAND_MANUFACTURER_ID_ATO			0x9B
#define _SPI_NAND_MANUFACTURER_ID_ATO_2			0xAD
#define _SPI_NAND_MANUFACTURER_ID_FM			0xA1
#define _SPI_NAND_MANUFACTURER_ID_XTX			0x0B
#define _SPI_NAND_MANUFACTURER_ID_MIRA			0xC8
#define _SPI_NAND_MANUFACTURER_ID_BIWIN			0xBC
#define _SPI_NAND_MANUFACTURER_ID_FORESEE		0xCD
#define _SPI_NAND_MANUFACTURER_ID_DS			0xE5
#define _SPI_NAND_MANUFACTURER_ID_FISON			0x6B
#define _SPI_NAND_MANUFACTURER_ID_TYM			0x19

/* SPI NAND Device ID */
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG	0xF1
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG	0xD1
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG	0xB1
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG	0xD2
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UEYIS	0xD3
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS	0xD5
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG	0xB2
#define _SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG	0xD4
#define _SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG	0xB4
#define _SPI_NAND_DEVICE_ID_GD5F1GQ5UEYIG	0x51
#define _SPI_NAND_DEVICE_ID_GD5F1GQ5REYIG	0x41
#define _SPI_NAND_DEVICE_ID_F50L512M41A		0x20
#define _SPI_NAND_DEVICE_ID_F50L1G41A0		0x21
#define _SPI_NAND_DEVICE_ID_F50L1G41LB		0x01
#define _SPI_NAND_DEVICE_ID_F50L2G41LB		0x0A
#define _SPI_NAND_DEVICE_ID_1_W25N01GV		0xAA
#define _SPI_NAND_DEVICE_ID_2_W25N01GV		0x21
#define _SPI_NAND_DEVICE_ID_1_W25N02KV		0xAA
#define _SPI_NAND_DEVICE_ID_2_W25N02KV		0x22
#define _SPI_NAND_DEVICE_ID_1_W25M02GV		0xAB
#define _SPI_NAND_DEVICE_ID_2_W25M02GV		0x21
#define _SPI_NAND_DEVICE_ID_MXIC35LF1GE4AB	0x12
#define _SPI_NAND_DEVICE_ID_MXIC35LF2GE4AB	0x22
#define _SPI_NAND_DEVICE_ID_1_MXIC35LF2GE4AD	0x26
#define _SPI_NAND_DEVICE_ID_2_MXIC35LF2GE4AD	0x03
#define _SPI_NAND_DEVICE_ID_MXIC35LF4GE4AB	0x32
#define _SPI_NAND_DEVICE_ID_A5U12A21ASC		0x20
#define _SPI_NAND_DEVICE_ID_A5U1GA21BWS		0x21
#define _SPI_NAND_DEVICE_ID_EM73C044SNA		0x19
#define _SPI_NAND_DEVICE_ID_EM73C044SNB		0x11
#define _SPI_NAND_DEVICE_ID_EM73C044SND		0x1D
#define _SPI_NAND_DEVICE_ID_EM73C044SNF		0x09
#define _SPI_NAND_DEVICE_ID_EM73C044VCA		0x18
#define _SPI_NAND_DEVICE_ID_EM73C044VCD		0x1C
#define _SPI_NAND_DEVICE_ID_EM73D044SNA		0x12
#define _SPI_NAND_DEVICE_ID_EM73D044SNC		0x0A
#define _SPI_NAND_DEVICE_ID_EM73D044SND		0x1E
#define _SPI_NAND_DEVICE_ID_EM73D044SNF		0x10
#define _SPI_NAND_DEVICE_ID_EM73D044VCA		0x13
#define _SPI_NAND_DEVICE_ID_EM73D044VCB		0x14
#define _SPI_NAND_DEVICE_ID_EM73D044VCD		0x17
#define _SPI_NAND_DEVICE_ID_EM73D044VCG		0x1F
#define _SPI_NAND_DEVICE_ID_EM73D044VCH		0x1B
#define _SPI_NAND_DEVICE_ID_EM73E044SNA		0x03
#define _SPI_NAND_DEVICE_ID_TC58CVG0S3H		0xC2
#define _SPI_NAND_DEVICE_ID_TC58CVG1S3H		0xCB
#define _SPI_NAND_DEVICE_ID_TC58CVG2S0H		0xCD
#define _SPI_NAND_DEVICE_ID_TC58CVG2S0HRAIJ	0xED
#define _SPI_NAND_DEVICE_ID_MT29F1G01		0x14
#define _SPI_NAND_DEVICE_ID_MT29F2G01		0x24
#define _SPI_NAND_DEVICE_ID_MT29F4G01		0x36
#define _SPI_NAND_DEVICE_ID_HYF1GQ4UAACAE	0x51
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UAACAE	0x52
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UHCCAE	0x5A
#define _SPI_NAND_DEVICE_ID_HYF1GQ4UDACAE	0x21
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UDACAE	0x22
#define _SPI_NAND_DEVICE_ID_PN26G01AWSIUG	0xE1
#define _SPI_NAND_DEVICE_ID_PN26G02AWSIUG	0xE2
#define _SPI_NAND_DEVICE_ID_PN26Q01AWSIUG	0xC1
#define _SPI_NAND_DEVICE_ID_ATO25D1GA		0x12
#define _SPI_NAND_DEVICE_ID_ATO25D2GA		0xF1
#define _SPI_NAND_DEVICE_ID_ATO25D2GB		0xDA
#define _SPI_NAND_DEVICE_ID_FM25S01		0xA1
#define _SPI_NAND_DEVICE_ID_FM25S01A		0xE4
#define _SPI_NAND_DEVICE_ID_FM25G01B		0xD1
#define _SPI_NAND_DEVICE_ID_FM25G02B		0xD2
#define _SPI_NAND_DEVICE_ID_FM25G02		0xF2
#define _SPI_NAND_DEVICE_ID_FM25G02C		0x92
#define _SPI_NAND_DEVICE_ID_XT26G02B		0xF2
#define _SPI_NAND_DEVICE_ID_XT26G01C		0x11
#define _SPI_NAND_DEVICE_ID_XT26G01A		0xE1
#define _SPI_NAND_DEVICE_ID_XT26G02A		0xE2
#define _SPI_NAND_DEVICE_ID_PSU1GS20BN		0x21
#define _SPI_NAND_DEVICE_ID_BWJX08U		0xB1
#define _SPI_NAND_DEVICE_ID_BWET08U		0xB2
#define _SPI_NAND_DEVICE_ID_FS35ND01GD1F1	0xA1
#define _SPI_NAND_DEVICE_ID_FS35ND01GS1F1	0xB1
#define _SPI_NAND_DEVICE_ID_FS35ND02GS2F1	0xA2
#define _SPI_NAND_DEVICE_ID_FS35ND02GD1F1	0xB2
#define _SPI_NAND_DEVICE_ID_DS35Q2GA		0x72
#define _SPI_NAND_DEVICE_ID_DS35Q1GA		0x71
#define _SPI_NAND_DEVICE_ID_CS11G0T0A0AA	0x00
#define _SPI_NAND_DEVICE_ID_CS11G1T0A0AA	0x01
#define _SPI_NAND_DEVICE_ID_CS11G0G0A0AA	0x10
#define _SPI_NAND_DEVICE_ID_TYM25D2GA01		0x01
#define _SPI_NAND_DEVICE_ID_TYM25D2GA02		0x02
#define _SPI_NAND_DEVICE_ID_TYM25D1GA03		0x03

/* Others Define */
#define _SPI_NAND_LEN_ONE_BYTE			(1)
#define _SPI_NAND_LEN_TWO_BYTE			(2)
#define _SPI_NAND_LEN_THREE_BYTE		(3)
#define _SPI_NAND_BLOCK_ROW_ADDRESS_OFFSET	(6)

#define _SPI_NAND_OOB_SIZE			256
#define _SPI_NAND_PAGE_SIZE			(4096 + _SPI_NAND_OOB_SIZE)
#define _SPI_NAND_CACHE_SIZE			(_SPI_NAND_PAGE_SIZE+_SPI_NAND_OOB_SIZE)

#define LINUX_USE_OOB_START_OFFSET		4
#define MAX_LINUX_USE_OOB_SIZE			26

#define EMPTY_DATA				(0)
#define NONE_EMPTY_DATA				(1)
#define EMPTY_OOB				(0)
#define NONE_EMPTY_OOB				(1)

/* FUNCTION DECLARATIONS ------------------------------------------------------ */

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define _SPI_NAND_BLOCK_ALIGNED_CHECK( __addr__,__block_size__) ((__addr__) & ( __block_size__ - 1))
#define _SPI_NAND_GET_DEVICE_INFO_PTR		&(_current_flash_info_t)

/* Porting Replacement */
#define _SPI_NAND_SEMAPHORE_LOCK()			/* Disable interrupt */
#define _SPI_NAND_SEMAPHORE_UNLOCK()			/* Enable interrupt  */
#define _SPI_NAND_PRINTF			printf	/* Always print information */
#if !defined(SPI_NAND_FLASH_DEBUG)
#define _SPI_NAND_DEBUG_PRINTF(args...)
#define _SPI_NAND_DEBUG_PRINTF_ARRAY(args...)
#else
#define _SPI_NAND_DEBUG_PRINTF			/* spi_nand_flash_debug_printf */
#define _SPI_NAND_DEBUG_PRINTF_ARRAY		/* spi_nand_flash_debug_printf_array */
#endif
#define _SPI_NAND_ENABLE_MANUAL_MODE		SPI_CONTROLLER_Enable_Manual_Mode
#define _SPI_NAND_WRITE_ONE_BYTE		SPI_CONTROLLER_Write_One_Byte
#define _SPI_NAND_WRITE_NBYTE			SPI_CONTROLLER_Write_NByte
#define _SPI_NAND_READ_NBYTE			SPI_CONTROLLER_Read_NByte
#define _SPI_NAND_READ_CHIP_SELECT_HIGH		SPI_CONTROLLER_Chip_Select_High
#define _SPI_NAND_READ_CHIP_SELECT_LOW		SPI_CONTROLLER_Chip_Select_Low

int ECC_fcheck = 1;
int ECC_ignore = 0;
int OOB_size = 0;
int Skip_BAD_page = 0;

static unsigned char _plane_select_bit = 0;
static unsigned char _die_id = 0;
int en_oob_write = 0;
int en_oob_erase = 0;

unsigned char _ondie_ecc_flag = 1;    /* Ondie ECC : [ToDo :  Init this flag base on diffrent chip ?] */

#define IMAGE_OOB_SIZE				64	/* fix 64 oob buffer size padding after page buffer, no hw ecc info */
#define PAGE_OOB_SIZE				64	/* 64 bytes for 2K page, 128 bytes for 4k page */

#define BLOCK_SIZE				(_current_flash_info_t.erase_size)

/* STATIC VARIABLE DECLARATIONS ------------------------------------------------------ */
static unsigned long bmt_oob_size = PAGE_OOB_SIZE;
static u32 erase_oob_size = 0;
static u32 ecc_size = 0;
u32 bsize = 0;

static u32 _current_page_num = 0xFFFFFFFF;
static u8 _current_cache_page[_SPI_NAND_CACHE_SIZE];
static u8 _current_cache_page_data[_SPI_NAND_PAGE_SIZE];
static u8 _current_cache_page_oob[_SPI_NAND_OOB_SIZE];
static u8 _current_cache_page_oob_mapping[_SPI_NAND_OOB_SIZE];

static struct SPI_NAND_FLASH_INFO_T _current_flash_info_t;	/* Store the current flash information */

/*****************************[ Notice]******************************/
/* If new spi nand chip have page size more than 4KB,  or oob size more than 256 bytes,  than*/
/* it will need to adjust the #define of _SPI_NAND_PAGE_SIZE and _SPI_NAND_OOB_SIZE */
/*****************************[ Notice]******************************/

static const struct SPI_NAND_FLASH_INFO_T spi_nand_flash_tables[] = {
	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG,
		ptr_name:				"GIGADEVICE GD5F1GQ4UA",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG,
		ptr_name:				"GIGADEVICE GD5F1GQ4UB",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG,
		ptr_name:				"GIGADEVICE GD5F1GQ4UC",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ4UEYIS,
		ptr_name:				"GIGADEVICE GD5F1GQ4UE",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ5UEYIG,
		ptr_name:				"GIGADEVICE GD5F1GQ5UE",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F1GQ5REYIG,
		ptr_name:				"GIGADEVICE GD5F1GQ5RE",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG,
		ptr_name:				"GIGADEVICE GD5F2GQ4UB",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"GIGADEVICE GD5F2GQ4UE",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG,
		ptr_name:				"GIGADEVICE GD5F2GQ4UC",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG,
		ptr_name:				"GIGADEVICE GD5F4GQ4UB",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_4KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_256BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_256KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_GIGADEVICE,
		dev_id:					_SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG,
		ptr_name:				"GIGADEVICE GD5F4GQ4UC",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_4KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_256BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_256KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ESMT,
		dev_id:					_SPI_NAND_DEVICE_ID_F50L512M41A,
		ptr_name:				"ESMT F50L512",
		device_size:				_SPI_NAND_CHIP_SIZE_512MBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ESMT,
		dev_id:					_SPI_NAND_DEVICE_ID_F50L1G41A0,
		ptr_name:				"ESMT F50L1G",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ESMT,
		dev_id:					_SPI_NAND_DEVICE_ID_F50L1G41LB,
		ptr_name:				"ESMT F50L1G41LB",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ESMT,
		dev_id:					_SPI_NAND_DEVICE_ID_F50L2G41LB,
		ptr_name:				"ESMT F50L2G41LB",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_DIE_SELECT_1_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_WINBOND,
		dev_id:					_SPI_NAND_DEVICE_ID_1_W25N01GV,
		dev_id_2:				_SPI_NAND_DEVICE_ID_2_W25N01GV,
		ptr_name:				"WINBOND W25N01G",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_WINBOND,
		dev_id:					_SPI_NAND_DEVICE_ID_1_W25N02KV,
		dev_id_2:				_SPI_NAND_DEVICE_ID_2_W25N02KV,
		ptr_name:				"WINBOND W25N02KV",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_WINBOND,
		dev_id:					_SPI_NAND_DEVICE_ID_1_W25M02GV,
		dev_id_2:				_SPI_NAND_DEVICE_ID_2_W25M02GV,
		ptr_name:				"WINBOND W25M02G",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_DIE_SELECT_1_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MXIC,
		dev_id:					_SPI_NAND_DEVICE_ID_MXIC35LF1GE4AB,
		ptr_name:				"MXIC MX35LF1G",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MXIC,
		dev_id:					_SPI_NAND_DEVICE_ID_MXIC35LF2GE4AB,
		ptr_name:				"MXIC MX35LF2G",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_PLANE_SELECT_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MXIC,
		dev_id:					_SPI_NAND_DEVICE_ID_1_MXIC35LF2GE4AD,
		dev_id_2:				_SPI_NAND_DEVICE_ID_2_MXIC35LF2GE4AD,
		ptr_name:				"MXIC MX35LF2GE4AD",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_PLANE_SELECT_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ZENTEL,
		dev_id:					_SPI_NAND_DEVICE_ID_A5U12A21ASC,
		ptr_name:				"ZENTEL A5U12A21ASC",
		device_size:				_SPI_NAND_CHIP_SIZE_512MBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ZENTEL,
		dev_id:					_SPI_NAND_DEVICE_ID_A5U1GA21BWS,
		ptr_name:				"ZENTEL A5U1GA21BWS",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* Etron */
	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73C044SNB,
		ptr_name:				"ETRON EM73C044SNB",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73C044SND,
		ptr_name:				"ETRON EM73C044SND",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73C044SNF",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73C044SNF,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73C044VCA",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73C044VCA,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73C044VCD",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73C044VCD,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73D044VCA",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044VCA,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73D044VCB",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044VCB,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73D044VCD",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044VCD,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73D044VCG",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044VCG,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"ETRON EM73D044VCH",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044VCH,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044SNA,
		ptr_name:				"ETRON EM73D044SNA",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044SNC,
		ptr_name:				"ETRON EM73D044SNC",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044SND,
		ptr_name:				"ETRON EM73D044SND",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73D044SNF,
		ptr_name:				"ETRON EM73D044SNF",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ETRON,
		dev_id:					_SPI_NAND_DEVICE_ID_EM73E044SNA,
		ptr_name:				"ETRON EM73E044SNA",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_4KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_256BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_256KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_TOSHIBA,
		dev_id:					_SPI_NAND_DEVICE_ID_TC58CVG0S3H,
		ptr_name:				"TOSHIBA TC58CVG0S3H",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_TOSHIBA,
		dev_id:					_SPI_NAND_DEVICE_ID_TC58CVG1S3H,
		ptr_name:				"TOSHIBA TC58CVG1S3H",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_TOSHIBA,
		dev_id:					_SPI_NAND_DEVICE_ID_TC58CVG2S0H,
		ptr_name:				"TOSHIBA TC58CVG2S0H",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_4KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_256KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_TOSHIBA,
		dev_id:					_SPI_NAND_DEVICE_ID_TC58CVG2S0HRAIJ,
		ptr_name:				"KIOXIA TC58CVG2S0HRAIJ",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_4KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_256KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MICRON,
		dev_id:					_SPI_NAND_DEVICE_ID_MT29F1G01,
		ptr_name:				"MICRON MT29F1G01",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MICRON,
		dev_id:					_SPI_NAND_DEVICE_ID_MT29F2G01,
		ptr_name:				"MICRON MT29F2G01",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_PLANE_SELECT_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MICRON,
		dev_id:					_SPI_NAND_DEVICE_ID_MT29F4G01,
		ptr_name:				"MICRON MT29F4G01",
		device_size:				_SPI_NAND_CHIP_SIZE_4GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_PLANE_SELECT_HAVE | SPI_NAND_FLASH_DIE_SELECT_2_HAVE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_HEYANG,
		dev_id:					_SPI_NAND_DEVICE_ID_HYF1GQ4UAACAE,
		ptr_name:				"HEYANG HYF1GQ4UAACAE",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_HEYANG,
		dev_id:					_SPI_NAND_DEVICE_ID_HYF2GQ4UAACAE,
		ptr_name:				"HEYANG HYF2GQ4UAACAE",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_HEYANG,
		dev_id:					_SPI_NAND_DEVICE_ID_HYF2GQ4UHCCAE,
		ptr_name:				"HEYANG HYF2GQ4UHCCAE",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_HEYANG,
		dev_id:					_SPI_NAND_DEVICE_ID_HYF1GQ4UDACAE,
		ptr_name:				"HEYANG HYF1GQ4UDACAE",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_HEYANG,
		dev_id:					_SPI_NAND_DEVICE_ID_HYF2GQ4UDACAE,
		ptr_name:				"HEYANG HYF2GQ4UDACAE",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_PN,
		dev_id:					_SPI_NAND_DEVICE_ID_PN26G01AWSIUG,
		ptr_name:				"PN PN26G01A-X",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_PN,
		dev_id:					_SPI_NAND_DEVICE_ID_PN26G02AWSIUG,
		ptr_name:				"PN PN26G02A-X",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_PN,
		dev_id:					_SPI_NAND_DEVICE_ID_PN26Q01AWSIUG,
		ptr_name:				"PN PN26Q01A-X",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ATO,
		dev_id:					_SPI_NAND_DEVICE_ID_ATO25D1GA,
		ptr_name:				"ATO ATO25D1GA",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ATO,
		dev_id:					_SPI_NAND_DEVICE_ID_ATO25D2GA,
		ptr_name:				"ATO ATO25D2GA",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_ATO_2,
		dev_id:					_SPI_NAND_DEVICE_ID_ATO25D2GB,
		ptr_name:				"ATO ATO25D2GB",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* FM */
	{
		ptr_name:				"FM FM25S01",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25S01,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"FM FM25S01A",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25S01A,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25G01B,
		ptr_name:				"FM FM25G01B",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25G02B,
		ptr_name:				"FM FM25G02B",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25G02C,
		ptr_name:				"FM FM25G02C",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FM,
		dev_id:					_SPI_NAND_DEVICE_ID_FM25G02,
		ptr_name:				"FM FM25G02",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* XTX */
	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_XTX,
		dev_id:					_SPI_NAND_DEVICE_ID_XT26G02B,
		ptr_name:				"XTX XT26G02B",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"XTX XT26G01C",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_XTX,
		dev_id:					_SPI_NAND_DEVICE_ID_XT26G01C,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"XTX XT26G01A",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_XTX,
		dev_id:					_SPI_NAND_DEVICE_ID_XT26G01A,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"XTX XT26G02A",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_XTX,
		dev_id:					_SPI_NAND_DEVICE_ID_XT26G02A,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* Mira */
	{
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_MIRA,
		dev_id:					_SPI_NAND_DEVICE_ID_PSU1GS20BN,
		ptr_name:				"MIRA PSU1GS20BN",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* BIWIN */
	{
		ptr_name:				"BIWIN BWJX08U",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_BIWIN,
		dev_id:					_SPI_NAND_DEVICE_ID_BWJX08U,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"BIWIN BWET08U",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_BIWIN,
		dev_id:					_SPI_NAND_DEVICE_ID_BWET08U,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	/* FORESEE */
	{
		ptr_name:				"FORESEE FS35ND01GD1F1",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FORESEE,
		dev_id:					_SPI_NAND_DEVICE_ID_FS35ND01GD1F1,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"FORESEE FS35ND01GS1F1",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FORESEE,
		dev_id:					_SPI_NAND_DEVICE_ID_FS35ND01GS1F1,
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"FORESEE FS35ND02GS2F1",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FORESEE,
		dev_id:					_SPI_NAND_DEVICE_ID_FS35ND02GS2F1,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		ptr_name:				"FORESEE FS35ND02GD1F1",
		mfr_id:					_SPI_NAND_MANUFACTURER_ID_FORESEE,
		dev_id:					_SPI_NAND_DEVICE_ID_FS35ND02GD1F1,
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_128BYTE,
		erase_size:				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode:				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode:				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_DS,
		dev_id: 				_SPI_NAND_DEVICE_ID_DS35Q2GA,
		ptr_name:				"DS DS35Q2GA",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_PLANE_SELECT_HAVE,
	},
	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_DS,
		dev_id: 				_SPI_NAND_DEVICE_ID_DS35Q1GA,
		ptr_name:				"DS DS35Q1GA",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_FISON,
		dev_id: 				_SPI_NAND_DEVICE_ID_CS11G0T0A0AA,
		ptr_name:				"FISON CS11G0T0A0AA",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},
	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_FISON,
		dev_id: 				_SPI_NAND_DEVICE_ID_CS11G1T0A0AA,
		ptr_name:				"FISON CS11G1T0A0AA",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},
	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_FISON,
		dev_id: 				_SPI_NAND_DEVICE_ID_CS11G0G0A0AA,
		ptr_name:				"FISON CS11G0G0A0AA",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},

	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_TYM,
		dev_id: 				_SPI_NAND_DEVICE_ID_TYM25D2GA01,
		ptr_name:				"TYM TYM25D2GA01",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},
	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_TYM,
		dev_id: 				_SPI_NAND_DEVICE_ID_TYM25D2GA02,
		ptr_name:				"TYM TYM25D2GA02",
		device_size:				_SPI_NAND_CHIP_SIZE_2GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},
	{
		mfr_id: 				_SPI_NAND_MANUFACTURER_ID_TYM,
		dev_id: 				_SPI_NAND_DEVICE_ID_TYM25D1GA03,
		ptr_name:				"TYM TYM25D1GA03",
		device_size:				_SPI_NAND_CHIP_SIZE_1GBIT,
		page_size:				_SPI_NAND_PAGE_SIZE_2KBYTE,
		oob_size:				_SPI_NAND_OOB_SIZE_64BYTE,
		erase_size: 				_SPI_NAND_BLOCK_SIZE_128KBYTE,
		dummy_mode: 				SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
		read_mode:				SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
		write_mode: 				SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE,
		feature:				SPI_NAND_FLASH_FEATURE_NONE,
	},
};

/* LOCAL SUBPROGRAM BODIES------------------------------------------------------------ */
#if 0
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_reset( void )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send FFh opcode (Reset) */
	_SPI_NAND_WRITE_ONE_BYTE(_SPI_NAND_OP_RESET);

	/* 3. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_reset\n");

	return (rtn_status);
}
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_feature( u8 addr, u8 protection )
 * PURPOSE : To implement the SPI nand protocol for set status register.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - register address
 *           data - The variable of this register.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_feature( u8 addr, u8 data )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 0Fh opcode (Set Feature) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_SET_FEATURE );

	/* 3. Offset addr */
	_SPI_NAND_WRITE_ONE_BYTE( addr );

	/* 4. Write new setting */
	_SPI_NAND_WRITE_ONE_BYTE( data );

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_set_feature %x: val = 0x%x\n", addr, data);

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_feature( u8 addr, u8 *ptr_rtn_data )
 * PURPOSE : To implement the SPI nand protocol for get status register.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - register address
 *   OUTPUT: ptr_rtn_protection  - A pointer to the ptr_rtn_protection variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_feature( u8 addr, u8 *ptr_rtn_data )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 0Fh opcode (Get Feature) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_GET_FEATURE );

	/* 3. Offset addr */
	_SPI_NAND_WRITE_ONE_BYTE( addr );

	/* 4. Read 1 byte data */
	_SPI_NAND_READ_NBYTE( ptr_rtn_data, _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_get_feature %x: val = 0x%x\n", addr, *ptr_rtn_data);

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_1( u8    protection )
 * PURPOSE : To implement the SPI nand protocol for set status register 1.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : protection - The protection variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_1( u8 protection )
{
	/* Offset addr of protection (0xA0) */
	return spi_nand_protocol_set_feature(_SPI_NAND_ADDR_PROTECTION, protection);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_1( u8    *ptr_rtn_protection )
 * PURPOSE : To implement the SPI nand protocol for get status register 1.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_protection  - A pointer to the ptr_rtn_protection variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_1( u8 *ptr_rtn_protection )
{
	/* Offset addr of protection (0xA0) */
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_PROTECTION, ptr_rtn_protection);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_2( u8   feature )
 * PURPOSE : To implement the SPI nand protocol for set status register 2.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : feature - The feature variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_2( u8 feature )
{
	/* Offset addr of feature (0xB0) */
	return spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_2( u8   *ptr_rtn_feature )
 * PURPOSE : To implement the SPI nand protocol for get status register 2.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_feature  - A pointer to the ptr_rtn_feature variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_2( u8 *ptr_rtn_feature )
{
	/* Offset addr of protection (0xB0) */
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, ptr_rtn_feature);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_3( u8 *ptr_rtn_status )
 * PURPOSE : To implement the SPI nand protocol for get status register 3.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : stats_reg - The stats_reg variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_3( u8 *ptr_rtn_status )
{
	/* Offset addr of status (0xC0) */
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, ptr_rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_4( u8 feature )
 * PURPOSE : To implement the SPI nand protocol for set status register 4.
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : feature - The feature variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_status_reg_4( u8 feature )
{
	/* Offset addr of feature 4 (0xD0) */
	return spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE_4, feature);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_4( u8 *ptr_rtn_status )
 * PURPOSE : To implement the SPI nand protocol for get status register 4.
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_status  - A pointer to the ptr_rtn_status variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_4( u8 *ptr_rtn_status )
{
	/* Offset addr of feature 4 (0xD0) */
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE_4, ptr_rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_5( u8 *ptr_rtn_status )
 * PURPOSE : To implement the SPI nand protocol for get status register 5.
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_status  - A pointer to the ptr_rtn_status variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static inline SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_status_reg_5( u8 *ptr_rtn_status )
{
	/* Offset addr of status 5 (0xE0)) */
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS_5, ptr_rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_enable( void )
 * PURPOSE : To implement the SPI nand protocol for write enable.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_enable( void )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0x06 (Write Enable (WREN)*/
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_WRITE_ENABLE );

	/* 3. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_disable( void )
 * PURPOSE : To implement the SPI nand protocol for write disable.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_disable( void )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0x04 (Write Disable (WRDI)*/
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_WRITE_DISABLE );

	/* 3. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_block_erase( u32   block_idx )
 * PURPOSE : To implement the SPI nand protocol for block erase.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : block_idx - The block_idx variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_block_erase( u32 block_idx )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0xD8 (Block Erase) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_BLOCK_ERASE );

	/* 3. Write block number */
	block_idx = block_idx << _SPI_NAND_BLOCK_ROW_ADDRESS_OFFSET;	/*Row Address format in SPI NAND chip */

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_block_erase : block idx = 0x%x\n", block_idx);

	_SPI_NAND_WRITE_ONE_BYTE( (block_idx >> 16) & 0xff );		/* dummy byte */
	_SPI_NAND_WRITE_ONE_BYTE( (block_idx >> 8)  & 0xff );
	_SPI_NAND_WRITE_ONE_BYTE(  block_idx & 0xff );

	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
 * PURPOSE : To implement the SPI nand protocol for read id.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id ( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0x9F (Read ID) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_ID );

	/* 3. Write Address Byte (0x00) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_ADDR_MANUFACTURE_ID );

	/* 4. Read data (Manufacture ID and Device ID) */
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->mfr_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->dev_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->dev_id_2), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_id : mfr_id = 0x%x, dev_id = 0x%x, dev_id_2 = 0x%x\n",
			       ptr_rtn_flash_id->mfr_id, ptr_rtn_flash_id->dev_id, ptr_rtn_flash_id->dev_id_2);
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_2( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
 * PURPOSE : To implement the SPI nand protocol for read id.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_2 ( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0x9F (Read ID) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_ID );

	/* 3. Read data (Manufacture ID and Device ID) */
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->mfr_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->dev_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->dev_id_2), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);

	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_id_2 : mfr_id = 0x%x, dev_id = 0x%x, dev_id_2 = 0x%x\n",
			       ptr_rtn_flash_id->mfr_id, ptr_rtn_flash_id->dev_id, ptr_rtn_flash_id->dev_id_2);

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_3( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
 * PURPOSE : To implement the SPI nand protocol for read id.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   : Workaround Toshiba/KIOXIA
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_3 ( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u8 dummy = 0;

	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Write op_cmd 0x9F (Read ID) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_ID );

	/* 3. Read data (Manufacture ID and Device ID) */
	_SPI_NAND_READ_NBYTE( &dummy, _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->mfr_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	_SPI_NAND_READ_NBYTE( &(ptr_rtn_flash_id->dev_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);

	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_id_3 : dummy = 0x%x, mfr_id = 0x%x, dev_id = 0x%x\n", dummy, ptr_rtn_flash_id->mfr_id, ptr_rtn_flash_id->dev_id);

	return (rtn_status);
}


/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_page_read( u32    page_number )
 * PURPOSE : To implement the SPI nand protocol for page read.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : page_number - The page_number variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_page_read ( u32 page_number )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u8 cmd[4];

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 13h opcode */
	cmd[0] = _SPI_NAND_OP_PAGE_READ;

	/* 3. Send page number */
	cmd[1] = (page_number >> 16 ) & 0xff;
	cmd[2] = (page_number >> 8  ) & 0xff;
	cmd[3] = (page_number)        & 0xff;
	_SPI_NAND_WRITE_NBYTE( cmd, 4, SPI_CONTROLLER_SPEED_SINGLE );

	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: value = 0x%x\n", (page_number ) );

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_from_cache( u32  data_offset,
 *                                                                          u32  len,
 *                                                                          u8   *ptr_rtn_buf,
 *                                                                          u32  read_mode,
 *                                                                          SPI_NAND_FLASH_READ_DUMMY_BYTE_T dummy_mode)
 * PURPOSE : To implement the SPI nand protocol for read from cache with single speed.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : data_offset  - The data_offset variable of this function.
 *           len          - The len variable of this function.
 *   OUTPUT: ptr_rtn_buf  - A pointer to the ptr_rtn_buf variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_from_cache( u32 data_offset, u32 len, u8 *ptr_rtn_buf, u32 read_mode,
										SPI_NAND_FLASH_READ_DUMMY_BYTE_T dummy_mode )
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
#if 0
	/* 2. Send opcode */
	switch (read_mode)
	{
		/* 03h */
		case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_SINGLE );
			break;

		/* 3Bh */
		case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_DUAL );
			break;

		/* 6Bh */
		case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_QUAD );
			break;

		default:
			break;
	}
#else
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_SINGLE );
#endif
	/* 3. Send data_offset addr */
	if( dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND )
	{
		_SPI_NAND_WRITE_ONE_BYTE( (0xff) );	/* dummy byte */
	}

	if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
	{
		if( _plane_select_bit == 0)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) &(0xef)) );
		}
		if( _plane_select_bit == 1)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) | (0x10)) );
		}
	}
	else
	{
		_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) &(0xff)) );
	}

	_SPI_NAND_WRITE_ONE_BYTE( ((data_offset      ) &(0xff)) );

	if( dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND )
	{
		_SPI_NAND_WRITE_ONE_BYTE( (0xff) );	/* dummy byte */
	}

	if( dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND && 
	  ((read_mode == SPI_NAND_FLASH_READ_SPEED_MODE_DUAL) || (read_mode == SPI_NAND_FLASH_READ_SPEED_MODE_QUAD)))
	{
		_SPI_NAND_WRITE_ONE_BYTE(0xff);		/* for dual/quad read dummy byte */
	}

	/* 4. Read n byte (len) data */
	switch (read_mode)
	{
		case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_SINGLE);
			break;

		case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_DUAL);
			break;

		case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_QUAD);
			break;

		default:
			break;
	}

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_from_cache : data_offset = 0x%x, buf = 0x%x\n", data_offset, ptr_rtn_buf);	

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load( u32     addr,
 *                                                                       u8      *ptr_data,
 *                                                                       u32     len,
 *                                                                       u32 write_mode)
 * PURPOSE : To implement the SPI nand protocol for program load, with single speed.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr      - The addr variable of this function.
 *           ptr_data  - A pointer to the ptr_data variable.
 *           len       - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load ( u32 addr, u8 *ptr_data, u32 len, u32 write_mode)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_program_load: addr = 0xl%x, len = 0x%x\n", addr, len );

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
#if 0
	/* 2. Send opcode */
	switch (write_mode)
	{
		/* 02h */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_SINGLE );
			break;

		/* 32h */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_QUAD );
			break;

		default:
			break;
	}
#else
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_SINGLE );
#endif
	/* 3. Send address offset */
	if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
	{
		if( _plane_select_bit == 0)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) & (0xef)) );
		}
		if( _plane_select_bit == 1)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) | (0x10)) );
		}
	}
	else
	{
		_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) & (0xff)) );
	}

	_SPI_NAND_WRITE_ONE_BYTE( ((addr)        & (0xff)) );

	/* 4. Send data */
	switch (write_mode)
	{
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_SINGLE);
			break;

		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_QUAD);
			break;

		default:
			break;
	}
	
	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_execute( u32  addr )
 * PURPOSE : To implement the SPI nand protocol for program execute.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_execute ( u32 addr )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_program_execute: addr = 0x%x\n", addr);

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 10h opcode */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_EXECUTE );

	/* 3. Send address offset */
	_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 16  ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8   ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((addr)         & 0xff) );

	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_protocol_die_select_1( u8 die_id)
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send C2h opcode (Die Select) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_DIE_SELECT );

	/* 3. Send Die ID */
	_SPI_NAND_WRITE_ONE_BYTE( die_id );

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_die_select_1\n");

	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_protocol_die_select_2( u8 die_id)
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u8 feature;

	rtn_status = spi_nand_protocol_get_status_reg_4(&feature);
	if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		_SPI_NAND_PRINTF("spi_nand_protocol_die_select_2 get die select fail.\n");
		return (rtn_status);
	}

	if(die_id == 0) {
		feature &= ~(0x40);
	} else {
		feature |= 0x40;
	}
	rtn_status = spi_nand_protocol_set_status_reg_4(feature);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_die_select_2\n");

	return (rtn_status);
}

static void spi_nand_select_die ( u32 page_number )
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u8 die_id = 0;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) ) {
		/* single die = 1024blocks * 64pages */
		die_id = ((page_number >> 16) & 0xff);

		if (_die_id != die_id)
		{
			_die_id = die_id;
			spi_nand_protocol_die_select_1(die_id);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_1: die_id=0x%x\n", die_id);
		}
	} else if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE) ) {
		/* single die = 2plans * 1024blocks * 64pages */
		die_id = ((page_number >> 17) & 0xff);

		if (_die_id != die_id)
		{
			_die_id = die_id;
			spi_nand_protocol_die_select_2(die_id);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_2: die_id=0x%x\n", die_id);
		}
	}
}

static SPI_NAND_FLASH_RTN_T ecc_fail_check( u32 page_number )
{
	u8 status;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	spi_nand_protocol_get_status_reg_3( &status);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "ecc_fail_check: status = 0x%x\n", status);

	if((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) &&
		((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UEYIS) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5UEYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5REYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG)))
	{
		if((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UEYIS) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5UEYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5REYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG))
		{
			if(((status & 0x30) >> 4) == 0x2 )
			{
				rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
			}
		}

		if((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG) ||
			(ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG))
		{
			if(((status & 0x70) >> 4) == 0x7)
			{
				rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
			}
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_MXIC)
	{
		if(((status & 0x30) >> 4) == 0x2 )
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND)
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ESMT) &&
		((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_F50L512M41A) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_F50L1G41A0) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_F50L1G41LB) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_F50L2G41LB)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ZENTEL) &&
		((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_A5U12A21ASC) ||
		 (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_A5U1GA21BWS)))
	{
		if(((status & 0x30) >> 4) == 0x2 )
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ETRON)
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TOSHIBA)
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_MICRON)
	{
		if(((status & 0x70) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_HEYANG)
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_PN26G01AWSIUG)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_PN26G02AWSIUG)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_PN26Q01AWSIUG)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ATO) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_ATO25D2GA)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ATO_2) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_ATO25D2GB)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FM25S01)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FM25S01A)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G01B)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G02B)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G02C)))
	{
		if(((status & 0x70) >> 4) == 0x7)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G02B)))
	{
		if(((status & 0x70) >> 4) == 0x7)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if (((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G01C)))
	{
		if(((status & 0xF0) >> 4) == 0xF)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if (((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G01A)))
	{
		if(((status & 0x3C) >> 2) == 0x8)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if (((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G02A)))
	{
		if(((status & 0x30) >> 4) == 0x2 )
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_MIRA) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_PSU1GS20BN)))
	{
		if(((status & 0x30) >> 4) == 0x2 )
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if (((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_BIWIN) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_BWJX08U)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_BIWIN) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_BWET08U)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if (((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND02GS2F1)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND02GD1F1)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND01GS1F1)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND01GD1F1)))
	{
		if(((status & 0x70) >> 4) == 0x7)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_DS) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_DS35Q2GA)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_DS) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_DS35Q1GA)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G0T0A0AA)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G1T0A0AA)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G0G0A0AA)))
	{
		if(((status & 0x70) >> 4) == 0x7)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}
	else if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D2GA01)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D2GA02)) ||
		((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D1GA03)))
	{
		if(((status & 0x30) >> 4) == 0x2)
		{
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}
	}

	if(rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK)
	{
		_SPI_NAND_PRINTF("[spinand_ecc_fail_check] : ECC cannot recover detected !, page = 0x%x\n", page_number);
	}

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_load_page_into_cache( long  page_number )
 * PURPOSE : To load page into SPI NAND chip.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : page_number - The page_number variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_load_page_into_cache( u32 page_number)
{
	u8 status;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: page number = 0x%x\n", page_number);

	if( _current_page_num == page_number )
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: page number == _current_page_num\n");
	}
	else
	{
		spi_nand_select_die ( page_number );

		spi_nand_protocol_page_read ( page_number );

		/*  Checking status for load page/erase/program complete */
		do {
			spi_nand_protocol_get_status_reg_3( &status);
		} while( status & _SPI_NAND_VAL_OIP) ;

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache : status = 0x%x\n", status);
		if (ECC_fcheck && !ECC_ignore)
			rtn_status = ecc_fail_check(page_number);
		else
			rtn_status = 0;
	}

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: spi_nand_set_clock_speed( u32 clock_factor)
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : clock_factor - The SPI clock divider.
 * RETURN  : NONE.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static void spi_nand_set_clock_speed( u32 clk)
{

}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_block_aligned_check( u32   addr,
 *                                                                     u32   len  )
 * PURPOSE : To check block align.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *           len  - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_block_aligned_check( u32 addr, u32 len )
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR ;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: addr = 0x%x, len = 0x%x, block size = 0x%x \n", addr, len, (ptr_dev_info_t->erase_size));

	if (_SPI_NAND_BLOCK_ALIGNED_CHECK(len, (ptr_dev_info_t->erase_size))) 
	{
		len = ( (len/ptr_dev_info_t->erase_size) + 1) * (ptr_dev_info_t->erase_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: erase block aligned first check OK, addr:%x len:%x\n", addr, len, (ptr_dev_info_t->erase_size));
	}

	if (_SPI_NAND_BLOCK_ALIGNED_CHECK(addr, (ptr_dev_info_t->erase_size)) || _SPI_NAND_BLOCK_ALIGNED_CHECK(len, (ptr_dev_info_t->erase_size)) ) 
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: erase block not aligned, addr:0x%x len:0x%x, blocksize:0x%x\n", addr, len, (ptr_dev_info_t->erase_size));
		rtn_status = SPI_NAND_FLASH_RTN_ALIGNED_CHECK_FAIL;
	}

	return (rtn_status);
}

SPI_NAND_FLASH_RTN_T spi_nand_erase_block ( u32 block_index)
{
	u8 status;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	spi_nand_select_die ( (block_index << _SPI_NAND_BLOCK_ROW_ADDRESS_OFFSET) );

	/* 2.2 Enable write_to flash */
	spi_nand_protocol_write_enable();

	/* 2.3 Erasing one block */
	spi_nand_protocol_block_erase( block_index );

	/* 2.4 Checking status for erase complete */
	do {
		spi_nand_protocol_get_status_reg_3( &status);
	} while( status & _SPI_NAND_VAL_OIP) ;

	/* 2.5 Disable write_flash */
	spi_nand_protocol_write_disable();

	/* 2.6 Check Erase Fail Bit */
	if( status & _SPI_NAND_VAL_ERASE_FAIL )
	{
		_SPI_NAND_PRINTF("spi_nand_erase_block : erase block fail, block = 0x%x, status = 0x%x\n", block_index, status);
		rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
	}

	return rtn_status;
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_erase_internal( u32     addr,
 *                                                                u32     len )
 * PURPOSE : To erase flash internally.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *           len - The size variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_erase_internal( u32 addr, u32 len )
{
	u32 block_index = 0;
	u32 erase_len = 0;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_erase_internal (in): addr = 0x%x, len = 0x%x\n", addr, len );
	_SPI_NAND_SEMAPHORE_LOCK();

	/* Switch to manual mode*/
	_SPI_NAND_ENABLE_MANUAL_MODE();

	SPI_NAND_Flash_Clear_Read_Cache_Data();

	/* 1. Check the address and len must aligned to NAND Flash block size */
	if( spi_nand_block_aligned_check( addr, len) == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		/* 2. Erase block one by one */
		while( erase_len < len )
		{
			/* 2.1 Caculate Block index */
			block_index = (addr/(_current_flash_info_t.erase_size));

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_erase_internal: addr = 0x%x, len = 0x%x, block_idx = 0x%x\n", addr, len, block_index );

			rtn_status = spi_nand_erase_block(block_index);

			/* 2.6 Check Erase Fail Bit */
			if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR)
			{
				_SPI_NAND_PRINTF("spi_nand_erase_internal : Erase Fail at addr = 0x%x, len = 0x%x, block_idx = 0x%x\n", addr, len, block_index);
				rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
			}

			/* 2.7 Erase next block if needed */
			addr		+= _current_flash_info_t.erase_size;
			erase_len	+= _current_flash_info_t.erase_size;
			if( timer_progress() )
			{
				printf("\bErase %d%% [%u] of [%u] bytes      ", 100 * (erase_len / 1024) / (len / 1024), erase_len, len);
				printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				fflush(stdout);
			}
		}
		printf("Erase 100%% [%u] of [%u] bytes      \n", erase_len, len);
	}
	else
	{
		rtn_status = SPI_NAND_FLASH_RTN_ALIGNED_CHECK_FAIL;
	}

	_SPI_NAND_SEMAPHORE_UNLOCK();

	return 	(rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_read_page (u32 page_number, SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u16 read_addr;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	/* read from read_addr index in the page */
	read_addr = 0;

	/* Switch to manual mode*/
	_SPI_NAND_ENABLE_MANUAL_MODE();

	/* 1. Load Page into cache of NAND Flash Chip */
	if( spi_nand_load_page_into_cache(page_number) == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK )
	{
		_SPI_NAND_PRINTF("spi_nand_read_page: Bad Block, ECC cannot recovery detected, page = 0x%x\n", page_number);
		rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
	}

	/* 2. Read whole data from cache of NAND Flash Chip */
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_page: curren_page_num = 0x%x, page_number = 0x%x\n", _current_page_num, page_number);

	/* No matter what status, we must read the cache data to dram */
	if((_current_page_num != page_number))
	{
		memset(_current_cache_page, 0x0, sizeof(_current_cache_page)); 

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: before read, _current_cache_page:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], _SPI_NAND_CACHE_SIZE);
		
		if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
		{
			_plane_select_bit = ((page_number >> 6)& (0x1));

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"spi_nand_read_page: plane select = 0x%x\n",  _plane_select_bit);
		}

		{
			spi_nand_protocol_read_from_cache(read_addr, ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)), &_current_cache_page[0], speed_mode, ptr_dev_info_t->dummy_mode );
		}

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: after read, _current_cache_page:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], _SPI_NAND_CACHE_SIZE);

		/* Divide read page into data segment and oob segment  */
		memcpy( &_current_cache_page_data[0], &_current_cache_page[0], (ptr_dev_info_t->page_size) );

		if(ECC_fcheck) {
			memcpy( &_current_cache_page_oob[0],  &_current_cache_page[(ptr_dev_info_t->page_size)], (ptr_dev_info_t->oob_size) );
			memcpy( &_current_cache_page_oob_mapping[0],  &_current_cache_page_oob[0], (ptr_dev_info_t->oob_size) );
		}

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)));
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page_oob:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob[0], (ptr_dev_info_t->oob_size));
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page_oob_mapping:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob_mapping[0], (ptr_dev_info_t->oob_size));

		_current_page_num = page_number;
	}

	return rtn_status;
}

static SPI_NAND_FLASH_RTN_T spi_nand_write_page( u32 page_number, u32 data_offset, u8  *ptr_data, u32 data_len, u32 oob_offset, u8  *ptr_oob,
											u32 oob_len, SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode )
{
		u8 status, status_2;
		struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
		SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
		u16 write_addr;

		int only_ffff = 1;

		for (int i=0; i<data_len; i++){
			if (ptr_data[i] != 0xff) {
				only_ffff = 0;
				break;
			}
		}

		if (only_ffff) {
			return 0;
		}

		/* write to write_addr index in the page */
		write_addr = 0;

		/* Switch to manual mode*/
		_SPI_NAND_ENABLE_MANUAL_MODE();

		ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

		/* Read Current page data to software cache buffer */
		rtn_status = spi_nand_read_page(page_number, speed_mode);
		if (Skip_BAD_page && (rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK)) { /* skip BAD page, go to next page */
			return SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}

		/* Rewirte the software cahe buffer */
		if(data_len > 0)
		{	
			memcpy( &_current_cache_page_data[data_offset], &ptr_data[0], data_len );
		}

		memcpy( &_current_cache_page[0], &_current_cache_page_data[0], ptr_dev_info_t->page_size);

		if(ECC_fcheck && oob_len > 0 )	/* Write OOB */
		{
			memcpy( &_current_cache_page_oob[0], &ptr_oob[0], oob_len);
			if(ptr_dev_info_t->oob_size)
				memcpy( &_current_cache_page[ptr_dev_info_t->page_size],  &_current_cache_page_oob[0], ptr_dev_info_t->oob_size );
		}

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: page = 0x%x, data_offset = 0x%x, date_len = 0x%x, oob_offset = 0x%x, oob_len = 0x%x\n", page_number, data_offset, data_len, oob_offset, oob_len);
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)));

		if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
		{
			_plane_select_bit = ((page_number >> 6) & (0x1));

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: _plane_select_bit = 0x%x\n", _plane_select_bit );
		}

		spi_nand_select_die ( page_number );

		/* Different Manafacture have different prgoram flow and setting */
		if( ((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_PN) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_FM) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_XTX) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_FORESEE) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_FISON) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_TYM) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_ATO_2) ||
			(((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_ATO) && ((ptr_dev_info_t->dev_id) == _SPI_NAND_DEVICE_ID_ATO25D2GA)))
		{
			{
				spi_nand_protocol_program_load(write_addr, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)), speed_mode);
			}

			/* Enable write_to flash */
			spi_nand_protocol_write_enable();
		}
		else
		{
			/* Enable write_to flash */
			spi_nand_protocol_write_enable();

			{
				/* Proram data into buffer of SPI NAND chip */
				spi_nand_protocol_program_load(write_addr, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)), speed_mode);
			}
		}

		/* Execute program data into SPI NAND chip  */
		spi_nand_protocol_program_execute ( page_number );

		/* Checking status for erase complete */
		do {
			spi_nand_protocol_get_status_reg_3( &status);
		} while( status & _SPI_NAND_VAL_OIP) ;

		/*. Disable write_flash */
		spi_nand_protocol_write_disable();

		spi_nand_protocol_get_status_reg_1( &status_2);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spi_nand_write_page]: status 1 = 0x%x, status 3 = 0x%x\n", status_2, status);
		/* Check Program Fail Bit */
		if( status & _SPI_NAND_VAL_PROGRAM_FAIL )
		{
			_SPI_NAND_PRINTF("spi_nand_write_page : Program Fail at addr_offset = 0x%x, page_number = 0x%x, status = 0x%x\n", data_offset, page_number, status);
			rtn_status = SPI_NAND_FLASH_RTN_PROGRAM_FAIL;
		}

		SPI_NAND_Flash_Clear_Read_Cache_Data();

		return (rtn_status);
}

int test_write_fail_flag = 0;

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_write_internal( u32    dst_addr,
 *                                                                u32    len,
 *                                                                u32    *ptr_rtn_len,
 *                                                                u8*    ptr_buf      )
 * PURPOSE : To write flash internally.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr     - The dst_addr variable of this function.
 *           len          - The len variable of this function.
 *           ptr_buf      - A pointer to the ptr_buf variable.
 *   OUTPUT: ptr_rtn_len  - A pointer to the ptr_rtn_len variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_write_internal( u32 dst_addr, u32 len, u32 *ptr_rtn_len, u8* ptr_buf, SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode )
{
	u32 remain_len, write_addr, data_len, page_number, physical_dst_addr;
	u32 addr_offset;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	*ptr_rtn_len = 0;
	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;
	remain_len = len;
	write_addr = dst_addr;

	_SPI_NAND_SEMAPHORE_LOCK();

	SPI_NAND_Flash_Clear_Read_Cache_Data();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_write_internal: remain_len = 0x%x\n", remain_len);

	while( remain_len > 0 )
	{
		physical_dst_addr = write_addr;

		/* Caculate page number */
		addr_offset = (physical_dst_addr % (ptr_dev_info_t->page_size));
		page_number = (physical_dst_addr / (ptr_dev_info_t->page_size));

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_write_internal: addr_offset = 0x%x, page_number = 0x%x, remain_len = 0x%x, page_size = 0x%x\n", addr_offset, page_number, remain_len,(ptr_dev_info_t->page_size) );		
		if( ((addr_offset + remain_len ) > (ptr_dev_info_t->page_size))  )  /* data cross over than 1 page range */
		{
			data_len = ((ptr_dev_info_t->page_size) - addr_offset);
		}
		else
		{
			data_len = remain_len;
		}

		rtn_status = spi_nand_write_page(page_number, addr_offset, &(ptr_buf[len - remain_len]), data_len, 0, NULL, 0 , speed_mode);
		/* skip BAD page or internal error on write page, go to next page */
		if( Skip_BAD_page && ((rtn_status == SPI_NAND_FLASH_RTN_PROGRAM_FAIL) || (rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK)) ) {
			if( ((addr_offset + remain_len ) < (ptr_dev_info_t->page_size)) )
				break;
			write_addr += data_len;
			continue;
		}

		/* 8. Write remain data if neccessary */
		write_addr += data_len;
		remain_len -= data_len;
		ptr_rtn_len += data_len;
		if( timer_progress() )
		{
			printf("\bWritten %d%% [%u] of [%u] bytes      ", 100 * ((len - remain_len) / 1024) / (len / 1024), len - remain_len, len);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}
	printf("Written 100%% [%u] of [%u] bytes      \n", len - remain_len, len);
	_SPI_NAND_SEMAPHORE_UNLOCK();

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_read_internal( u32     addr,
 *                                                               u32     len,
 *                                                               u8      *ptr_rtn_buf )
 * PURPOSE : To read flash internally.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr         - The addr variable of this function.
 *           len          - The len variable of this function.
 *   OUTPUT: ptr_rtn_buf  - A pointer to the ptr_rtn_buf variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_read_internal ( u32 addr, u32 len, u8 *ptr_rtn_buf, SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode,
									SPI_NAND_FLASH_RTN_T *status)
{
	u32 page_number, data_offset;
	u32 read_addr, physical_read_addr, remain_len;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	read_addr = addr;
	remain_len = len;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_read_internal : addr = 0x%lx, len = 0x%x\n", addr, len );

	_SPI_NAND_SEMAPHORE_LOCK();

	*status = SPI_NAND_FLASH_RTN_NO_ERROR;

	while(remain_len > 0)
	{
		physical_read_addr = read_addr;

		/* Caculate page number */
		data_offset = (physical_read_addr % (ptr_dev_info_t->page_size));
		page_number = (physical_read_addr / (ptr_dev_info_t->page_size));

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_internal: read_addr = 0x%x, page_number = 0x%x, data_offset = 0x%x\n", physical_read_addr, page_number, data_offset);

		rtn_status = spi_nand_read_page(page_number, speed_mode);
		if(rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK) {
			if (!Skip_BAD_page) {
				*status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
				return (rtn_status);
			}
			/* skip BAD page, go to next page */
			if( (data_offset + remain_len) < ptr_dev_info_t->page_size )
				break;
			read_addr += (ptr_dev_info_t->page_size - data_offset);
			continue;
		}

		/* 3. Retrieve the request data */
		if( (data_offset + remain_len) < ptr_dev_info_t->page_size )
		{
			memcpy( &ptr_rtn_buf[len - remain_len], &_current_cache_page_data[data_offset], (sizeof(unsigned char)*remain_len) );
			remain_len = 0;
		}
		else
		{
			memcpy( &ptr_rtn_buf[len - remain_len], &_current_cache_page_data[data_offset], (sizeof(unsigned char)*(ptr_dev_info_t->page_size - data_offset)));
			remain_len -= (ptr_dev_info_t->page_size - data_offset);
			read_addr += (ptr_dev_info_t->page_size - data_offset);
		}
		if( timer_progress() )
		{
			printf("\bRead %d%% [%u] of [%u] bytes      ", 100 * ((len - remain_len) / 1024) / (len / 1024), len - remain_len, len);
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			fflush(stdout);
		}
	}
	printf("Read 100%% [%u] of [%u] bytes      \n", len - remain_len, len);
	_SPI_NAND_SEMAPHORE_UNLOCK();

	return (rtn_status);
}


/*------------------------------------------------------------------------------------
 * FUNCTION: static void spi_nand_manufacute_init( struct SPI_NAND_FLASH_INFO_T *ptr_device_t )
 * PURPOSE : To init SPI NAND Flash chip
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None.
 * RETURN  : None.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static void spi_nand_manufacute_init( struct SPI_NAND_FLASH_INFO_T *ptr_device_t )
{
	unsigned char feature;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"SPI NAND Chip Init : Unlock all block and Enable Quad Mode\n"); 

	if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ4UEYIS)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5UEYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F1GQ5REYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_GIGADEVICE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC1;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);

		spi_nand_protocol_get_status_reg_2(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 = 0x%x\n", feature);
	}
	else if((ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_MXIC)
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC1;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);

		spi_nand_protocol_get_status_reg_2(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 = 0x%x\n", feature);
	}
	else if( (ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_WINBOND)
	{
		if(((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE)) {
			_die_id = 0x00;
			spi_nand_protocol_die_select_1(_die_id);
		}

		/* Enable to modify the status regsiter 1 */
		feature = 0x58;
		spi_nand_protocol_set_status_reg_2(feature);

		/* Unlock all block and Enable Qual mode */
		feature = 0x81;
		spi_nand_protocol_set_status_reg_1(feature);

		/* Disable to modify the status regsiter 1 */
		feature = 0x18;
		spi_nand_protocol_set_status_reg_2(feature);

		spi_nand_protocol_get_status_reg_1(&feature);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* Unlock all block for Die_1 */
		if( ((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) )
		{
			_die_id = 0x01;
			spi_nand_protocol_die_select_1(_die_id);

			/* Enable to modify the status regsiter 1 */
			feature = 0x58;
			spi_nand_protocol_set_status_reg_2(feature);

			/* Unlock all block and Enable Qual mode */
			feature = 0x81;
			spi_nand_protocol_set_status_reg_1(feature);

			/* Disable to modify the status regsiter 1 */
			feature = 0x18;
			spi_nand_protocol_set_status_reg_2(feature);

			spi_nand_protocol_get_status_reg_1(&feature);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the die %d status register1 = 0x%x\n", _die_id, feature);
		}
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ESMT) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_F50L512M41A)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ESMT) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_F50L1G41A0)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ESMT) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_F50L1G41LB)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ESMT) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_F50L2G41LB)))
	{
		if(((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE))
		{
			_die_id = 0x00;
			spi_nand_protocol_die_select_1(_die_id);
		}

		/* 1. Unlock All block */
		feature = 0x83;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* Unlock all block for Die_1 */
		if(((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE))
		{
			_die_id = 0x01;
			spi_nand_protocol_die_select_1(_die_id);

			/* 1. Unlock All block */
			feature = 0x83;
			spi_nand_protocol_set_status_reg_1(feature);

			spi_nand_protocol_get_status_reg_1(&feature);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the die %d status register1 = 0x%x\n", _die_id, feature);
		}
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ZENTEL) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_A5U12A21ASC)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_ZENTEL) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_A5U1GA21BWS)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else if( (ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_ETRON)
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC1;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);

		spi_nand_protocol_get_status_reg_2(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 = 0x%x\n", feature);
	}
	else if( (ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_TOSHIBA)
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}

	else if( (ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_MICRON)
	{
		if(((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE)) {
			_die_id = 0x00;
			spi_nand_protocol_die_select_2(_die_id);
		}

		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0x83;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* Unlock all block for Die_1 */
		if( ((ptr_device_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE) )
		{
			_die_id = 0x01;
			spi_nand_protocol_die_select_2(_die_id);

			/* 1. Unlock All block */
			spi_nand_protocol_get_status_reg_1(&feature);
			feature &= 0x83;
			spi_nand_protocol_set_status_reg_1(feature);

			spi_nand_protocol_get_status_reg_1(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"After Unlock all block setup, the die %d status register1 = 0x%x\n", _die_id, feature);
		}
	}
	else if( (ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_HEYANG)
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_PN26G01AWSIUG)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_PN26G02AWSIUG)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_PN) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_PN26Q01AWSIUG)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if( ((ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_ATO) ||
		 ((ptr_device_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_ATO_2) )
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FM25S01)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FM25S01A)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0x87;
		spi_nand_protocol_set_status_reg_1(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G01B)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G02B)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FM25G02C)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G02B)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_XTX) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_XT26G02A)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_MIRA) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_PSU1GS20BN)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_BIWIN) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_BWJX08U)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_BIWIN) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_BWET08U)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND02GS2F1)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND02GD1F1)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND01GS1F1)) ||
			((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FORESEE) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_FS35ND01GD1F1)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_DS) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_DS35Q2GA)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_DS) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_DS35Q1GA)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G0T0A0AA)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G1T0A0AA)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_FISON) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_CS11G0G0A0AA)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D2GA01)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D2GA02)) ||
		((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_TYM) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_TYM25D1GA03)))
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC7;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
	}
	else
	{
		/* 1. Unlock All block */
		spi_nand_protocol_get_status_reg_1(&feature);
		feature &= 0xC1;
		spi_nand_protocol_set_status_reg_1(feature);

		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);

		/* 2. Enable Qual mode */
		spi_nand_protocol_get_status_reg_2(&feature);
		feature |= 0x1;
		spi_nand_protocol_set_status_reg_2(feature);

		spi_nand_protocol_get_status_reg_2(&feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 = 0x%x\n", feature);
	}
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T spi_nand_compare( const struct SPI_NAND_FLASH_INFO_T *ptr_rtn_device_t,
 *						    const struct SPI_NAND_FLASH_INFO_T *spi_nand_flash_table )
 * PURPOSE : Compare a read SPI NAND flash ID with a flash table entry ID.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : ptr_rtn_device_t     - The pointer to a read SPI NAND flash description.
 *           spi_nand_flash_table - The pointer to a flash table entry.
 *   OUTPUT: None.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_compare( const struct SPI_NAND_FLASH_INFO_T *ptr_rtn_device_t,
					      const struct SPI_NAND_FLASH_INFO_T *spi_nand_flash_table )
{
	if ( spi_nand_flash_table->dev_id_2 == 0 )
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_compare: mfr_id = 0x%x, dev_id = 0x%x\n",
				       spi_nand_flash_table->.mfr_id, spi_nand_flash_table->.dev_id);

		if ( ( (ptr_rtn_device_t->mfr_id) == spi_nand_flash_table->mfr_id) &&
		     ( (ptr_rtn_device_t->dev_id) == spi_nand_flash_table->dev_id) )
		{
			return SPI_NAND_FLASH_RTN_NO_ERROR;
		}
	}
	else
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_compare: mfr_id = 0x%x, dev_id = 0x%x, dev_id_2 = 0x%x\n",
				       spi_nand_flash_table->.mfr_id, spi_nand_flash_table->dev_id, spi_nand_flash_table->.dev_id_2);

		if ( ( (ptr_rtn_device_t->mfr_id) == spi_nand_flash_table->mfr_id) &&
		     ( (ptr_rtn_device_t->dev_id) == spi_nand_flash_table->dev_id) &&
		     ( (ptr_rtn_device_t->dev_id_2) == spi_nand_flash_table->dev_id_2) )
		{
			return SPI_NAND_FLASH_RTN_NO_ERROR;
		}
	}

	return SPI_NAND_FLASH_RTN_PROBE_ERROR;
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_probe( struct SPI_NAND_FLASH_INFO_T  *ptr_rtn_device_t )
 * PURPOSE : To probe SPI NAND flash id.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: rtn_index  - The rtn_index variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_probe( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_device_t )
{
	u32 i = 0;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_PROBE_ERROR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: start \n");

	/* Protocol for read id */
	_SPI_NAND_SEMAPHORE_LOCK();
	spi_nand_protocol_read_id( ptr_rtn_device_t );
	_SPI_NAND_SEMAPHORE_UNLOCK();

	for ( i = 0; i < (sizeof(spi_nand_flash_tables)/sizeof(struct SPI_NAND_FLASH_INFO_T)); i++)
	{
		if ( spi_nand_compare( ptr_rtn_device_t, &spi_nand_flash_tables[i] ) == SPI_NAND_FLASH_RTN_NO_ERROR )
		{
			int oob_size = OOB_size ? OOB_size : spi_nand_flash_tables[i].oob_size;
			ecc_size = ((spi_nand_flash_tables[i].device_size / spi_nand_flash_tables[i].erase_size) * ((spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size));
			ptr_rtn_device_t->device_size = ECC_fcheck ? spi_nand_flash_tables[i].device_size : spi_nand_flash_tables[i].device_size + ecc_size;
			erase_oob_size                = (spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size;
			ptr_rtn_device_t->erase_size  = ECC_fcheck ? spi_nand_flash_tables[i].erase_size : spi_nand_flash_tables[i].erase_size + erase_oob_size;
			ptr_rtn_device_t->page_size   = ECC_fcheck ? spi_nand_flash_tables[i].page_size : spi_nand_flash_tables[i].page_size + oob_size;
			ptr_rtn_device_t->oob_size    = ECC_fcheck ? oob_size : 0;
			bmt_oob_size                  = spi_nand_flash_tables[i].oob_size;
			ptr_rtn_device_t->dummy_mode  = spi_nand_flash_tables[i].dummy_mode;
			ptr_rtn_device_t->read_mode   = spi_nand_flash_tables[i].read_mode;
			ptr_rtn_device_t->write_mode  = spi_nand_flash_tables[i].write_mode;
			memcpy( &(ptr_rtn_device_t->ptr_name) , &(spi_nand_flash_tables[i].ptr_name), sizeof(ptr_rtn_device_t->ptr_name));
			ptr_rtn_device_t->feature = spi_nand_flash_tables[i].feature;

			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
			break;
		}
	}

	if ( rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		/* Another protocol for read id  (For example, the GigaDevice SPI NADN chip for Type C */
		_SPI_NAND_SEMAPHORE_LOCK();
		spi_nand_protocol_read_id_2( ptr_rtn_device_t );
		_SPI_NAND_SEMAPHORE_UNLOCK();

		for ( i = 0; i < (sizeof(spi_nand_flash_tables) / sizeof(struct SPI_NAND_FLASH_INFO_T)); i++)
		{
			if ( spi_nand_compare( ptr_rtn_device_t, &spi_nand_flash_tables[i] ) == SPI_NAND_FLASH_RTN_NO_ERROR )
			{
				int oob_size = OOB_size ? OOB_size : spi_nand_flash_tables[i].oob_size;
				ecc_size = ((spi_nand_flash_tables[i].device_size / spi_nand_flash_tables[i].erase_size) * ((spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size));
				ptr_rtn_device_t->device_size = ECC_fcheck ? spi_nand_flash_tables[i].device_size : spi_nand_flash_tables[i].device_size + ecc_size;
				erase_oob_size                = (spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size;
				ptr_rtn_device_t->erase_size  = ECC_fcheck ? spi_nand_flash_tables[i].erase_size : spi_nand_flash_tables[i].erase_size + erase_oob_size;
				ptr_rtn_device_t->page_size   = ECC_fcheck ? spi_nand_flash_tables[i].page_size : spi_nand_flash_tables[i].page_size + oob_size;
				ptr_rtn_device_t->oob_size    = ECC_fcheck ? oob_size : 0;
				bmt_oob_size                  = spi_nand_flash_tables[i].oob_size;
				ptr_rtn_device_t->dummy_mode  = spi_nand_flash_tables[i].dummy_mode;
				ptr_rtn_device_t->read_mode   = spi_nand_flash_tables[i].read_mode;
				ptr_rtn_device_t->write_mode  = spi_nand_flash_tables[i].write_mode;
				memcpy( &(ptr_rtn_device_t->ptr_name) , &(spi_nand_flash_tables[i].ptr_name), sizeof(ptr_rtn_device_t->ptr_name));
				ptr_rtn_device_t->feature = spi_nand_flash_tables[i].feature;

				rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
				break;
			}
		}
	}

	if ( rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		/* Another protocol for read id  (For example, the Toshiba/KIOXIA SPI NADN chip */
		_SPI_NAND_SEMAPHORE_LOCK();
		spi_nand_protocol_read_id_3( ptr_rtn_device_t );
		_SPI_NAND_SEMAPHORE_UNLOCK();

		for ( i = 0; i < (sizeof(spi_nand_flash_tables) / sizeof(struct SPI_NAND_FLASH_INFO_T)); i++)
		{
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"spi_nand_probe: table[%d]: mfr_id = 0x%x, dev_id = 0x%x\n", i, spi_nand_flash_tables[i].mfr_id, spi_nand_flash_tables[i].dev_id );

			if ( ( (ptr_rtn_device_t->mfr_id) == spi_nand_flash_tables[i].mfr_id) &&
			     ( (ptr_rtn_device_t->dev_id) == spi_nand_flash_tables[i].dev_id)  )
			{
				int oob_size = OOB_size ? OOB_size : spi_nand_flash_tables[i].oob_size;
				ecc_size = ((spi_nand_flash_tables[i].device_size / spi_nand_flash_tables[i].erase_size) * ((spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size));
				ptr_rtn_device_t->device_size = ECC_fcheck ? spi_nand_flash_tables[i].device_size : spi_nand_flash_tables[i].device_size + ecc_size;
				erase_oob_size                = (spi_nand_flash_tables[i].erase_size / spi_nand_flash_tables[i].page_size) * spi_nand_flash_tables[i].oob_size;
				ptr_rtn_device_t->erase_size  = ECC_fcheck ? spi_nand_flash_tables[i].erase_size : spi_nand_flash_tables[i].erase_size + erase_oob_size;
				ptr_rtn_device_t->page_size   = ECC_fcheck ? spi_nand_flash_tables[i].page_size : spi_nand_flash_tables[i].page_size + oob_size;
				ptr_rtn_device_t->oob_size    = ECC_fcheck ? oob_size : 0;
				bmt_oob_size                  = spi_nand_flash_tables[i].oob_size;
				ptr_rtn_device_t->dummy_mode  = spi_nand_flash_tables[i].dummy_mode;
				ptr_rtn_device_t->read_mode   = spi_nand_flash_tables[i].read_mode;
				ptr_rtn_device_t->write_mode  = spi_nand_flash_tables[i].write_mode;
				memcpy( &(ptr_rtn_device_t->ptr_name) , &(spi_nand_flash_tables[i].ptr_name), sizeof(ptr_rtn_device_t->ptr_name));
				ptr_rtn_device_t->feature = spi_nand_flash_tables[i].feature;

				rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
				break;
			}
		}
	}

	if ( ptr_rtn_device_t->dev_id_2 == 0 )
	{
		_SPI_NAND_PRINTF("spi_nand_probe: mfr_id = 0x%x, dev_id = 0x%x\n", ptr_rtn_device_t->mfr_id, ptr_rtn_device_t->dev_id);
	}
	else
	{
		_SPI_NAND_PRINTF("spi_nand_probe: mfr_id = 0x%x, dev_id = 0x%x, dev_id_2 = 0x%x\n",
				 ptr_rtn_device_t->mfr_id, ptr_rtn_device_t->dev_id, ptr_rtn_device_t->dev_id_2);
	}

	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		unsigned char feature = 0;
		_SPI_NAND_SEMAPHORE_LOCK();
		spi_nand_protocol_get_status_reg_1(&feature);
		_SPI_NAND_PRINTF("Get Status Register 1: 0x%02x\n", feature);
		spi_nand_protocol_get_status_reg_2(&feature);
		_SPI_NAND_PRINTF("Get Status Register 2: 0x%02x\n", feature);
		spi_nand_manufacute_init(ptr_rtn_device_t);
		_SPI_NAND_SEMAPHORE_UNLOCK();
	}

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: end \n");

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init( long  rom_base )
 * PURPOSE : To provide interface for SPI NAND init.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : rom_base - The rom_base variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init(u32 rom_base)
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_PROBE_ERROR;

	/* 1. set SFC Clock to 50MHZ  */
	spi_nand_set_clock_speed(50);

	/* 2. Enable Manual Mode */
	_SPI_NAND_ENABLE_MANUAL_MODE();

	/* 3. Probe flash information */
	if ( spi_nand_probe(  &_current_flash_info_t) != SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		_SPI_NAND_PRINTF("SPI NAND Flash Not Detected!\n");
	}
	else
	{

		if(ECC_fcheck) {
			_SPI_NAND_PRINTF("Using Flash ECC.\n");
		} else {
			_SPI_NAND_PRINTF("Disable Flash ECC.\n");
			if(OOB_size > bmt_oob_size) {
				_SPI_NAND_PRINTF("Setting OOB size %dB cannot be larger %ldB!\n", OOB_size, bmt_oob_size);
				return SPI_NAND_FLASH_RTN_PROBE_ERROR;
			}
			if(OOB_size)
				_SPI_NAND_PRINTF("OOB Resize: %ldB to %dB.\n", bmt_oob_size, OOB_size);
		}
		SPI_NAND_Flash_Enable_OnDie_ECC();
		_SPI_NAND_PRINTF("Detected SPI NAND Flash: %s, Flash Size: %dMB, OOB Size: %ldB\n", _current_flash_info_t.ptr_name,  ECC_fcheck ? _current_flash_info_t.device_size >> 20 : (_current_flash_info_t.device_size - ecc_size) >> 20, OOB_size ? OOB_size : bmt_oob_size);

		rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	}

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info( struct SPI_NAND_FLASH_INFO_T    *ptr_rtn_into_t )
 * PURPOSE : To get system current flash info.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_into_t  - A pointer to the structure of the ptr_rtn_into_t variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	memcpy( ptr_rtn_into_t, ptr_dev_info_t, sizeof(struct SPI_NAND_FLASH_INFO_T) );

	return (rtn_status);
}

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Set_Flash_Info( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	memcpy(ptr_dev_info_t, ptr_rtn_into_t, sizeof(struct SPI_NAND_FLASH_INFO_T) );

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32    dst_addr,
 *                                                            u32    len,
 *                                                            u32    *ptr_rtn_len,
 *                                                            u8*    ptr_buf      )
 * PURPOSE : To provide interface for Write N Bytes into SPI NAND Flash.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *           buf      - The buf variable of this function.
 *   OUTPUT: rtn_len  - The rtn_len variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32 dst_addr, u32 len, u32 *ptr_rtn_len, u8 *ptr_buf,
						SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_node )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	rtn_status = spi_nand_write_internal(dst_addr, len, ptr_rtn_len, ptr_buf, speed_node);

	*ptr_rtn_len = len ;

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: int SPI_NAND_Flash_Read_NByte( long     addr,
 *                                          long     len,
 *                                          long     *retlen,
 *                                          char     *buf    )
 * PURPOSE : To provide interface for Read N Bytes from SPI NAND Flash.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr    - The addr variable of this function.
 *           len     - The len variable of this function.
 *           retlen  - The retlen variable of this function.
 *           buf     - The buf variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
u32 SPI_NAND_Flash_Read_NByte(u32  addr, u32  len, u32  *retlen, u8 *buf, SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode,
						SPI_NAND_FLASH_RTN_T *status)
{
	return spi_nand_read_internal(addr, len, buf, speed_mode, status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase( u32  dst_addr,
 *                                                      u32  len      )
 * PURPOSE : To provide interface for Erase SPI NAND Flash.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase( u32 dst_addr, u32 len )
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	rtn_status = spi_nand_erase_internal(dst_addr, len);

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: char SPI_NAND_Flash_Read_Byte( long     addr )
 * PURPOSE : To provide interface for read 1 Bytes from SPI NAND Flash.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
unsigned char SPI_NAND_Flash_Read_Byte(unsigned long addr, SPI_NAND_FLASH_RTN_T *status)
{
	u32 len = 1;
	u8 buf[2];
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	memset(buf,0x0,2);

	spi_nand_read_internal(addr, len, &buf[0], ptr_dev_info_t->read_mode, status);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_Byte : buf = 0x%x\n", buf[0]);

	return buf[0];
}

/*------------------------------------------------------------------------------------
 * FUNCTION: long SPI_NAND_Flash_Read_DWord( long    addr )
 * PURPOSE : To provide interface for read Double Word from SPI NAND Flash.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
unsigned long SPI_NAND_Flash_Read_DWord(unsigned long addr, SPI_NAND_FLASH_RTN_T *status)
{
	u8 buf2[4] = { 0 };
	u32 ret_val = 0;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_DWord, addr = 0x%llx\n", addr);

	spi_nand_read_internal(addr, 4, &buf2[0], ptr_dev_info_t->read_mode, status);
	ret_val = (buf2[0] << 24) | (buf2[1] << 16) | (buf2[2] <<8) | buf2[3];

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_DWord : ret_val = 0x%x\n", ret_val);

	return ret_val;
}

/*------------------------------------------------------------------------------------
 * FUNCTION: void SPI_NAND_Flash_Clear_Read_Cache_Data( void )
 * PURPOSE : To clear the cache data for read.
 *           (The next time to read data will get data from flash chip certainly.)
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
void SPI_NAND_Flash_Clear_Read_Cache_Data( void )
{
	_current_page_num = 0xFFFFFFFF;
}

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Enable_OnDie_ECC( void )
{
	unsigned char feature;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u8 die_num;
	int i;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if(((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE)) {
		die_num = (ptr_dev_info_t->device_size / ptr_dev_info_t->page_size) >> 16;

		for(i = 0; i < die_num; i++) {
			spi_nand_protocol_die_select_1(i);

			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
			if (ECC_fcheck)
				feature |= 0x10;
			else
				feature &= ~(1 << 4);
			spi_nand_protocol_set_status_reg_2(feature);

			/* Value check*/
			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
		}
	} else if(((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE)) {
		die_num = (ptr_dev_info_t->device_size / ptr_dev_info_t->page_size) >> 17;

		for(i = 0; i < die_num; i++) {
			spi_nand_protocol_die_select_2(i);

			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
			if (ECC_fcheck)
				feature |= 0x10;
			else
				feature &= ~(1 << 4);
			spi_nand_protocol_set_status_reg_2(feature);

			/* Value check*/
			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
		}
	} else {
		if( ((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_PN) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_FM) ||
			((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_FORESEE) ||
			(((ptr_dev_info_t->mfr_id) == _SPI_NAND_MANUFACTURER_ID_XTX) && ((ptr_dev_info_t->dev_id) == _SPI_NAND_DEVICE_ID_XT26G02B)) )
		{
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_ECC, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, ecc reg = 0x%x\n", feature);
			if (ECC_fcheck)
				feature |= 0x10;
			else
				feature &= ~(1 << 4);
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_ECC, feature);

			/* Value check*/
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_ECC, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, ecc reg = 0x%x\n", feature);
		}
		else
		{
			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
			
			if (ECC_fcheck)
				feature |= 0x10;
			else
				feature &= ~(1 << 4);
			spi_nand_protocol_set_status_reg_2(feature);

			/* Value check*/
			spi_nand_protocol_get_status_reg_2(&feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg = 0x%x\n", feature);
		}
	}

	if (ECC_fcheck)
		_ondie_ecc_flag = 1;
	else
		_ondie_ecc_flag = 0;
	return (SPI_NAND_FLASH_RTN_NO_ERROR);
}

int nandflash_init(int rom_base)
{
	if( SPI_NAND_Flash_Init(rom_base) == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int nandflash_read(unsigned long from, unsigned long len, unsigned long *retlen, unsigned char *buf, SPI_NAND_FLASH_RTN_T *status)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	timer_start();
	if( SPI_NAND_Flash_Read_NByte(from, len, (u32 *)retlen, buf, ptr_dev_info_t->read_mode, status) == SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		timer_end();
		return 0;
	}
	else
	{
		return -1;
	}
}

int nandflash_erase(unsigned long offset, unsigned long len)
{
	timer_start();
	if( SPI_NAND_Flash_Erase(offset, len) == SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		timer_end();
		return 0;
	}
	else
	{
		return -1;
	}
}

int nandflash_write(unsigned long to, unsigned long len, unsigned long *retlen, unsigned char *buf)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	timer_start();
	if( SPI_NAND_Flash_Write_Nbyte(to, len, (u32 *)retlen, buf, ptr_dev_info_t->write_mode) == SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		timer_end();
		return 0;
	}
	else
	{
		return -1;
	}
}
/* End of [spi_nand_flash.c] package */

int snand_read(unsigned char *buf, unsigned long from, unsigned long len)
{
	unsigned long retlen = 0;
	SPI_NAND_FLASH_RTN_T status;

	if(!nandflash_read((unsigned long)from, (unsigned long)len, &retlen, buf, &status))
		return len;
	return -1;
}

int snand_erase(unsigned long offs, unsigned long len)
{
	return nandflash_erase((unsigned long)offs, (unsigned long)len);
}

int snand_write(unsigned char *buf, unsigned long to, unsigned long len)
{
	unsigned long retlen = 0;

	if(!nandflash_write((unsigned long)to, (unsigned long)len, &retlen, buf))
		return (int)retlen;
	return -1;
}

long snand_init(void)
{
	if(!nandflash_init(0)) {
		struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;
		bsize = ptr_dev_info_t->erase_size;
		return (long)(ptr_dev_info_t->device_size);
	}
	return -1;
}

void support_snand_list(void)
{
	int i;

	_SPI_NAND_PRINTF("SPI NAND Flash Support List:\n");
	for ( i = 0; i < (sizeof(spi_nand_flash_tables)/sizeof(struct SPI_NAND_FLASH_INFO_T)); i++)
	{
		_SPI_NAND_PRINTF("%03d. %s\n", i + 1, spi_nand_flash_tables[i].ptr_name);
	}
}

/* End of [spi_nand_flash.c] package */
