// libUSB driver for the ch347a in i2c mode
//
// Copyright 2011 asbokid <ballymunboy@gmail.com>
// (c) December 2023 aystarik <aystarik@gmail.com>

#pragma once
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#define ch347TOOLVERSION            "0.6"

#define USB_LOCK_VENDOR             0x1a86      // Dev : (1a86) QinHeng Electronics
#define USB_LOCK_PRODUCT            0x5512      //       (5512) ch347A in i2c mode
#define USB_LOCK_PRODUCT2           0x55db      //       (5512) ch347A in i2c mode

#define EEPROM_I2C_BUS_ADDRESS      0x50

#define BULK_WRITE_ENDPOINT         0x06        /* bEndpointAddress 0x02  EP 2 OUT (Bulk) */
#define BULK_READ_ENDPOINT          0x86        /* bEndpointAddress 0x82  EP 2 IN  (Bulk) */
#define DEFAULT_INTERFACE           0x02

#define DEFAULT_CONFIGURATION       0x01
#define DEFAULT_TIMEOUT             3000 // 300mS for USB timeouts

/* Based on (closed-source) DLL V1.9 for USB by WinChipHead (c) 2005.
   Supports USB chips: ch347, ch347A
   This can be a problem for copyright, sure asbokid can't release this part on any GPL licence*/

#define	mch347_PACKET_LENGTH		32      /* wMaxPacketSize 0x0020  1x 32 bytes, unused on the source */
#define	mch347_PKT_LEN_SHORT		8       /* wMaxPacketSize 0x0008  1x 8 bytes, unused on the source */

#define	mch347_ENDP_INTER_UP		0x81    /* bEndpointAddress 0x81  EP 1 IN (Interrupt), unused on the source */
#define	mch347_ENDP_INTER_DOWN		0x01    /* This endpoint isn't list on my lsusb -v output, unused on the source */
#define	mch347_ENDP_DATA_UP		0x82    /* ==BULK_READ_ENDPOINT  Why repeat it? */
#define	mch347_ENDP_DATA_DOWN		0x02    /* ==BULK_WRITE_ENDPOINT Why repeat it? */

#define	mch347_VENDOR_READ		0xC0    /* Unused on the source */
#define	mch347_VENDOR_WRITE		0x40    /* Unused on the source */

#define	mch347_PARA_INIT		0xB1    /* Unused on the source */
#define	mch347_I2C_STATUS		0x52    /* Unused on the source */
#define	mch347_I2C_COMMAND		0x53    /* Unused on the source */

#define	mch347_PARA_CMD_R0		0xAC    /* Unused on the source */
#define	mch347_PARA_CMD_R1		0xAD    /* Unused on the source */
#define	mch347_PARA_CMD_W0		0xA6    /* Unused on the source */
#define	mch347_PARA_CMD_W1		0xA7    /* Unused on the source */
#define	mch347_PARA_CMD_STS		0xA0    /* Unused on the source */

#define	mch347A_CMD_SET_OUTPUT		0xA1    /* Unused on the source */
#define	mch347A_CMD_IO_ADDR		0xA2    /* Unused on the source */
#define	mch347A_CMD_PRINT_OUT		0xA3    /* Unused on the source */
#define	mch347A_CMD_SPI_STREAM		0xA8    /* Unused on the source */
#define	mch347A_CMD_SIO_STREAM		0xA9    /* Unused on the source */
#define	mch347A_CMD_I2C_STREAM		0xAA
#define	mch347A_CMD_UIO_STREAM		0xAB    /* Unused on the source */

#define	mch347A_BUF_CLEAR		0xB2    /* Unused on the source */
#define	mch347A_I2C_CMD_X		0x54    /* Unused on the source */
#define	mch347A_DELAY_MS		0x5E    /* Unused on the source */
#define	mch347A_GET_VER			0x5F    /* Unused on the source */

#define	mch347_EPP_IO_MAX		( mch347_PACKET_LENGTH - 1 )    /* Unused on the source */
#define	mch347A_EPP_IO_MAX		0xFF    /* Unused on the source */

#define	mch347A_CMD_IO_ADDR_W		0x00    /* Unused on the source */
#define	mch347A_CMD_IO_ADDR_R		0x80    /* Unused on the source */

#define	mch347A_CMD_I2C_STM_STA		0x74
#define	mch347A_CMD_I2C_STM_STO		0x75
#define	mch347A_CMD_I2C_STM_OUT		0x80
#define	mch347A_CMD_I2C_STM_IN		0xC0
#define	mch347A_CMD_I2C_STM_MAX		( min( 0x3F, mch347_PACKET_LENGTH ) )   /* Unused on the source */
#define	mch347A_CMD_I2C_STM_SET		0x60
#define	mch347A_CMD_I2C_STM_US		0x40    /* Unused on the source */
#define	mch347A_CMD_I2C_STM_MS		0x50    /* Unused on the source */
#define	mch347A_CMD_I2C_STM_DLY		0x0F    /* Unused on the source */
#define	mch347A_CMD_I2C_STM_END		0x00

#define	mch347A_CMD_UIO_STM_IN		0x00    /* Unused on the source */
#define	mch347A_CMD_UIO_STM_DIR		0x40    /* Unused on the source */
#define	mch347A_CMD_UIO_STM_OUT		0x80    /* Unused on the source */
#define	mch347A_CMD_UIO_STM_US		0xC0    /* Unused on the source */
#define	mch347A_CMD_UIO_STM_END		0x20    /* Unused on the source */

#define	mch347_PARA_MODE_EPP		0x00    /* Unused on the source */
#define	mch347_PARA_MODE_EPP17		0x00    /* Unused on the source */
#define	mch347_PARA_MODE_EPP19		0x01    /* Unused on the source */
#define	mch347_PARA_MODE_MEM		0x02    /* Unused on the source */

/* End of part based on (closed-source) DLL V1.9 for USB by WinChipHead (c) 2005.
   Since is largely unused we can replace it*/

enum ch347_speed_t {
    ch347_I2C_LOW_SPEED = 0,   // low speed - 20kHz
    ch347_I2C_STANDARD_SPEED = 1,      // standard speed - 100kHz
    ch347_I2C_FAST_SPEED = 2,  // fast speed - 400kHz
    ch347_I2C_HIGH_SPEED = 3,  // high speed - 750kHz
};

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct EEPROM2 {
    char *name;
    uint32_t size;
    uint16_t page_size;
    uint8_t addr_size;          // Length of addres in bytes
    uint8_t i2c_addr_mask;
};

//const static struct EEPROM eepromlist[] = {
//    { "24c01",      0x80,   8, 1, 0 },        // 16 pages of 8 bytes each = 128 bytes
//    { "24c02",     0x100,   8, 1, 0 },        // 32 pages of 8 bytes each = 256 bytes
//    { "24c04",     0x200,  16, 1, 1 },        // 32 pages of 16 bytes each = 512 bytes
//    { "24c08",     0x400,  16, 1, 3 },        // 64 pages of 16 bytes each = 1024 bytes
//    { "24c16",     0x800,  16, 1, 7 },        // 128 pages of 16 bytes each = 2048 bytes
//    { "24c32",    0x1000,  32, 2, 0 },        // 32kbit = 4kbyte
//    { "24c64",    0x2000,  32, 2, 0 },
//    { "24c128",   0x4000,  64, 2, 0 },
//    { "24c256",   0x8000,  64, 2, 0 },
//    { "24c512",  0x10000, 128, 2, 0 },
//    { "24c1024", 0x20000, 128, 2, 1 },
//    { "24cm02",  0x40000, 256, 2, 3 },
//    { 0, 0, 0, 0 }
//};

struct i2c_msg {
        uint16_t addr;     /* slave address                        */
        uint16_t flags;
#define I2C_M_RD                0x0001  /* read data, from slave to master */
                                        /* I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_TEN               0x0010  /* this is a ten bit chip address */
#define I2C_M_DMA_SAFE          0x0200  /* the buffer of this message is DMA safe */
                                        /* makes only sense in kernelspace */
                                        /* userspace buffers are copied anyway */
#define I2C_M_RECV_LEN          0x0400  /* length will be first received byte */
#define I2C_M_NO_RD_ACK         0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK        0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR      0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART           0x4000  /* if I2C_FUNC_NOSTART */
#define I2C_M_STOP              0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING */
        uint16_t len;              /* msg length                           */
        uint8_t *buf;              /* pointer to msg data                  */
};

int ch347i2cOpen();
int ch347i2cClose();
int32_t ch347readEEPROM( uint8_t *buffer, uint32_t startaddr, uint32_t ic_size, uint16_t blocksize, uint8_t algorithm);
int32_t ch347writeEEPROM( uint8_t * buf, uint32_t bytes, const struct EEPROM2 *eeprom_info);
struct libusb_device_handle *ch347configure(uint16_t vid, uint16_t pid);
int32_t ch347setstream( uint32_t speed);
int ch347_quick_write( uint8_t addr);
//int32_t parseEEPsize(char *eepromname, const struct EEPROM **eeprom);
int ch347i2cBlockRead(uint8_t * buf, uint32_t address, uint32_t blockSize, uint8_t algorithm);
int ch347i2cBlockWrite(uint8_t * buf, uint32_t address, uint32_t blockSize, uint8_t algorithm);
