// SPDX-License-Identifier: BSD-1-Clause
/*
 * Copyright (C) 2022 Chuanhong Guo <gch981213@gmail.com>
 * Copyright (C) 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 * CH347 SPI library using libusb. Protocol reverse-engineered from WCH linux library.
 * FIXME: Every numbers used in the USB protocol should be little-endian.
 */

#ifndef CH347_H
#define CH347_H

#ifdef __cplusplus
extern "C" {
#endif

#include <endian.h>
#include <stdint.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

#define CH347_SPI_VID 0x1a86
#define CH347_SPI_PID 0x55db
#define CH347_SPI_IF 2
#define CH347_EPOUT (6 | LIBUSB_ENDPOINT_OUT)
#define CH347_EPIN (6 | LIBUSB_ENDPOINT_IN)

#define CH347_SPI_MAX_FREQ 60000
#define CH347_SPI_MAX_PRESCALER 7
#define CH347_SPI_MAX_TRX 507//4096- wrong data. 510 - 3 = 507 - correct value

/* SPI_data_direction */
#define SPI_Direction_2Lines_FullDuplex 0x0000
#define SPI_Direction_2Lines_RxOnly 0x0400
#define SPI_Direction_1Line_Rx 0x8000
#define SPI_Direction_1Line_Tx 0xC000

/* SPI_mode */
#define SPI_Mode_Master 0x0104
#define SPI_Mode_Slave 0x0000

/* SPI_data_size */
#define SPI_DataSize_16b 0x0800
#define SPI_DataSize_8b 0x0000

/* SPI_Clock_Polarity */
#define SPI_CPOL_Low 0x0000
#define SPI_CPOL_High 0x0002

/* SPI_Clock_Phase */
#define SPI_CPHA_1Edge 0x0000
#define SPI_CPHA_2Edge 0x0001

/* SPI_Slave_Select_management */
#define SPI_NSS_Software 0x0200
#define SPI_NSS_Hardware 0x0000

/* SPI_MSB_LSB_transmission */
#define SPI_FirstBit_MSB 0x0000
#define SPI_FirstBit_LSB 0x0080

/* CH347 commands */
#define CH347_CMD_SPI_INIT 0xC0
#define CH347_CMD_SPI_CONTROL 0xC1
#define CH347_CMD_SPI_RD_WR 0xC2
#define CH347_CMD_SPI_BLCK_RD 0xC3
#define CH347_CMD_SPI_BLCK_WR 0xC4
#define CH347_CMD_INFO_RD 0xCA

#define	mch347A_CMD_I2C_STREAM		0xAA
#define	mch347A_CMD_I2C_STM_STA		0x74
#define	mch347A_CMD_I2C_STM_STO		0x75
#define	mch347A_CMD_I2C_STM_OUT		0x80
#define	mch347A_CMD_I2C_STM_IN		0xC0
#define	mch347A_CMD_I2C_STM_SET		0x60
#define	mch347A_CMD_I2C_STM_END		0x00

#define BULK_WRITE_ENDPOINT         0x06

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

struct ch347_spi_hw_config {
    uint16_t SPI_Direction;
    uint16_t SPI_Mode;
    uint16_t SPI_DataSize;
    uint16_t SPI_CPOL;
    uint16_t SPI_CPHA;
    uint16_t SPI_NSS; /* hardware or software managed CS */
    uint16_t SPI_BaudRatePrescaler; /* prescaler = x * 8. x: 0=60MHz, 1=30MHz, 2=15MHz, 3=7.5MHz, 4=3.75MHz, 5=1.875MHz, 6=937.5KHz，7=468.75KHz */
    uint16_t SPI_FirstBit; /* MSB or LSB first */
    uint16_t SPI_CRCPolynomial; /* polynomial used for the CRC calculation. */
    uint16_t SPI_WriteReadInterval; /* No idea what this is... Original comment from WCH: SPI接口常规读取写入数据命令(DEF_CMD_SPI_RD_WR))，单位为uS */
    uint8_t SPI_OutDefaultData;     /* Data to output on MOSI during SPI reading */
    /*
     * Miscellaneous settings:
     * Bit 7: CS0 polarity
     * Bit 6: CS1 polarity
     * Bit 5: Enable I2C clock stretching
     * Bit 4: NACK on last I2C reading
     * Bit 3-0: reserved
     */
    uint8_t OtherCfg;

    uint8_t Reserved[4];
};

struct ch347_priv {
    struct ch347_spi_hw_config cfg;
    libusb_context *ctx;
    libusb_device_handle *handle;
    uint8_t tmpbuf[510];//tmpbuf[509];//tmpbuf[8192];  // old - 512
};

struct ch347_priv *ch347_open();

void ch347_close(struct ch347_priv *priv);

int ch347_commit_settings(struct ch347_priv *priv);

int ch347_set_cs(struct ch347_priv *priv, int cs, int val);

int ch347_set_spi_freq(struct ch347_priv *priv, int *clk_khz);

int ch347_setup_spi(struct ch347_priv *priv, int spi_mode, bool lsb_first, bool cs0_active_high, bool cs1_active_high);

int ch347_spi_trx_full_duplex(struct ch347_priv *priv, void *buf, uint32_t len);

int ch347_spi_tx(struct ch347_priv *priv, const void *tx, uint32_t len);

int ch347_spi_rx(struct ch347_priv *priv, void *rx, uint32_t len);

bool ch347_spi_init(uint8_t ch_type, uint8_t i2cBusSpeed);

void ch347_spi_shutdown();

#ifdef __cplusplus
}
#endif

#endif //CH347_H
