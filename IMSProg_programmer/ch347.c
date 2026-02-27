// SPDX-License-Identifier: BSD-1-Clause
/*
 * Copyright (C) 2022 Chuanhong Guo <gch981213@gmail.com>
 * Copyright (C) 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 * CH347 SPI library using libusb. Protocol reverse-engineered from WCH linux library.
 * FIXME: Every numbers used in the USB protocol should be little-endian.
 */

#include "ch347.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ch347eeprom.h"

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#error You need to convert every USB communications to little endian before this library would work.
#endif

struct ch347_priv *priv = NULL;

int ch347_spi_write_packet(struct ch347_priv *priv, uint8_t cmd, const void *tx, int len) {
    uint8_t *ptr;
    int cur_len;
    int err, transferred;
    if (len > CH347_SPI_MAX_TRX)
        return -EINVAL;

    priv->tmpbuf[0] = cmd;
    priv->tmpbuf[1] = len & 0xff;
    priv->tmpbuf[2] = len >> 8;
    cur_len = sizeof(priv->tmpbuf) - 3;
    if (len < cur_len)
        cur_len = len;
    memcpy(priv->tmpbuf + 3, tx, cur_len);
    err = libusb_bulk_transfer(priv->handle, CH347_EPOUT, priv->tmpbuf, cur_len + 3, &transferred, 1000);
    if (err) {
        fprintf(stderr, "ch347: libusb: failed to send packet: %d\n", err);
        return err;
    }
    if (cur_len < len) {
        /* This discards the const qualifier. However, libusb won't be writing to it. */
        ptr = (uint8_t *) (tx + cur_len);
        err = libusb_bulk_transfer(priv->handle, CH347_EPOUT, ptr, len - cur_len, &transferred, 1000);
        if (err) {
            fprintf(stderr, "ch347: libusb: failed to send packet: %d\n", err);
            return err;
        }
    }
    return 0;
}

int ch347_spi_read_packet(struct ch347_priv *priv, uint8_t cmd, void *rx, int len, int *actual_len) {
    int cur_len, rxlen, rx_received;
    int err, transferred;

    err = libusb_bulk_transfer(priv->handle, CH347_EPIN, priv->tmpbuf, sizeof(priv->tmpbuf), &transferred, 1000);
    if (err) {
        fprintf(stderr, "ch347: libusb: failed to receive packet: %d\n", err);
        return err;
    }

    if (priv->tmpbuf[0] != cmd) {
        fprintf(stderr, "ch347: unexpected packet cmd: expecting 0x%02x but we got 0x%02x.\n", cmd, priv->tmpbuf[0]);
        return -EINVAL;
    }

    rxlen = priv->tmpbuf[1] | priv->tmpbuf[2] << 8;
    if (rxlen > len) {
        fprintf(stderr, "ch347: packet too big.\n");
        return -EINVAL;
    }

    cur_len = transferred - 3;
    if (rxlen < cur_len)
        cur_len = rxlen;
    memcpy(rx, priv->tmpbuf + 3, cur_len);
    rx_received = cur_len;
    while (rx_received < rxlen) {
        /* The leftover data length is known so we don't need to deal with packet overflow using tmpbuf. */
        err = libusb_bulk_transfer(priv->handle, CH347_EPIN, rx + rx_received, rxlen - rx_received, &transferred, 1000);
        if (err) {
            fprintf(stderr, "ch347: libusb: failed to receive packet: %d\n", err);
            return err;
        }
        rx_received += transferred;
    }

    *actual_len = rx_received;
    return 0;
}


int ch347_commit_settings(struct ch347_priv *priv) {
    int err, transferred;
    uint8_t unknown_data;
    err = ch347_spi_write_packet(priv, CH347_CMD_SPI_INIT, &priv->cfg, sizeof(priv->cfg));
    if (err)
        return err;

    return ch347_spi_read_packet(priv, CH347_CMD_SPI_INIT, &unknown_data, 1, &transferred);
}

int ch347_get_hw_config(struct ch347_priv *priv) {
    int err, transferred;
    uint8_t unknown_data = 0x01;

    err = ch347_spi_write_packet(priv, CH347_CMD_INFO_RD, &unknown_data, 1);
    if (err)
        return err;

    err = ch347_spi_read_packet(priv, CH347_CMD_INFO_RD, &priv->cfg, sizeof(priv->cfg), &transferred);
    if (err)
        return err;

    if (transferred != sizeof(priv->cfg)) {
        fprintf(stderr, "ch347: config returned isn't long enough.\n");
        return -EINVAL;
    }

    return 0;
}


int ch347_set_cs(struct ch347_priv *priv, int cs, int val) {
    uint8_t pkt_val = val ? 0xc0 : 0x80;
    uint8_t buf[10] = {};

    if (cs)
        buf[5] = pkt_val;
    else
        buf[0] = pkt_val;

    return ch347_spi_write_packet(priv, CH347_CMD_SPI_CONTROL, buf, 10);
}

int ch347_set_spi_freq(struct ch347_priv *priv, int *clk_khz) {
    int freq = CH347_SPI_MAX_FREQ;
    int prescaler;
    for (prescaler = 0; prescaler < CH347_SPI_MAX_PRESCALER; prescaler++) {
        if (freq <= *clk_khz)
            break;
        freq /= 2;
    }
    if (freq > *clk_khz)
        return -EINVAL;
    priv->cfg.SPI_BaudRatePrescaler = prescaler * 8;
    *clk_khz = freq;
    return ch347_commit_settings(priv);
}

int ch347_setup_spi(struct ch347_priv *priv, int spi_mode, bool lsb_first, bool cs0_active_high, bool cs1_active_high) {
    priv->cfg.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    priv->cfg.SPI_Mode = SPI_Mode_Master;
    priv->cfg.SPI_DataSize = SPI_DataSize_8b;
    priv->cfg.SPI_CPOL = (spi_mode & 2) ? SPI_CPOL_High : SPI_CPOL_Low;
    priv->cfg.SPI_CPHA = (spi_mode & 1) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    priv->cfg.SPI_NSS = SPI_NSS_Software;
    priv->cfg.SPI_FirstBit = lsb_first ? SPI_FirstBit_LSB : SPI_FirstBit_MSB;
    priv->cfg.SPI_WriteReadInterval = 0;
    priv->cfg.SPI_OutDefaultData = 0;

    if (cs0_active_high)
        priv->cfg.OtherCfg |= 0x80;
    else
        priv->cfg.OtherCfg &= 0x7f;
    if (cs1_active_high)
        priv->cfg.OtherCfg |= 0x40;
    else
        priv->cfg.OtherCfg &= 0xbf;

    return ch347_commit_settings(priv);
}

static int ch347_spi_trx_full_duplex_one(struct ch347_priv *priv, void *buf, uint32_t len) {
    int err, transferred;

    err = ch347_spi_write_packet(priv, CH347_CMD_SPI_RD_WR, buf, len);
    if (err)
        return err;

    err = ch347_spi_read_packet(priv, CH347_CMD_SPI_RD_WR, buf, len, &transferred);
    if (err)
        return err;

    if (transferred != len) {
        fprintf(stderr, "ch347: not enough data received.");
        return -EINVAL;
    }
    return 0;
}

int ch347_spi_trx_full_duplex(struct ch347_priv *priv, void *buf, uint32_t len) {
    int err;
    while (len > CH347_SPI_MAX_TRX) {
        err = ch347_spi_trx_full_duplex_one(priv, buf, CH347_SPI_MAX_TRX);
        if (err)
            return err;
        len -= CH347_SPI_MAX_TRX;
    }
    return ch347_spi_trx_full_duplex_one(priv, buf, len);
}

int ch347_spi_tx(struct ch347_priv *priv, const void *tx, uint32_t len) {
    int err, transferred;
    uint8_t unknown_data;
    const void *ptr = tx;
    while (len) {
        int cur_len = len > CH347_SPI_MAX_TRX ? CH347_SPI_MAX_TRX : len;
        err = ch347_spi_write_packet(priv, CH347_CMD_SPI_BLCK_WR, ptr, cur_len);
        if (err)
            return err;
        err = ch347_spi_read_packet(priv, CH347_CMD_SPI_BLCK_WR, &unknown_data, 1, &transferred);
        if (err)
            return err;
        ptr += cur_len;
        len -= cur_len;
    }
    return 0;
}

int ch347_spi_rx(struct ch347_priv *priv, void *rx, uint32_t len) {
    int err, transferred;
    void *ptr = rx;
    uint32_t rxlen = 0;
    /* FIXME: len should be little endian! */
    err = ch347_spi_write_packet(priv, CH347_CMD_SPI_BLCK_RD, &len, sizeof(len));
    if (err)
        return err;
    while(rxlen < len) {
        uint32_t cur_rx = len - rxlen;
        if(cur_rx > CH347_SPI_MAX_TRX)
            cur_rx = CH347_SPI_MAX_TRX;
        err = ch347_spi_read_packet(priv, CH347_CMD_SPI_BLCK_RD, ptr, (int)cur_rx, &transferred);
        if (err)
            return err;
        rxlen += transferred;
        ptr += transferred;
    }
    return 0;
}

/*   Set the I2C bus speed (speed(b1b0): 0 = 20kHz; 1 = 100kHz, 2 = 400kHz, 3 = 750kHz).*/

int32_t ch347setI2Cstream(uint32_t speed)
{
    int32_t actuallen = 0;
    uint8_t buf[3];

    buf[0] = mch347A_CMD_I2C_STREAM;
    buf[1] = mch347A_CMD_I2C_STM_SET | (speed & 0x3);
    buf[2] = mch347A_CMD_I2C_STM_END;

    int ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, buf, 3, &actuallen, 3000);

    if (ret < 0) {
        fprintf(stderr, "ch347setstream(): Failed write %d bytes '%s'\n", 2, strerror(-ret));
        return -1;
    }

    return 0;
}

struct ch347_priv *ch347_open() {
    struct ch347_priv *priv = calloc(1, sizeof(struct ch347_priv));
    int ret;

    if (!priv) {
        fprintf(stderr, "ch347: faied to allocate memory.\n");
        return NULL;
    }
    ret = libusb_init(&priv->ctx);
    if (ret < 0) {
        perror("ch347: libusb: init");
        goto ERR_0;
    }

    libusb_set_option(priv->ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
    priv->handle = libusb_open_device_with_vid_pid(priv->ctx, CH347_SPI_VID, CH347_SPI_PID);
    if (!priv->handle) {
        perror("ch347: libusb: open");
        goto ERR_1;
    }

    libusb_set_auto_detach_kernel_driver(priv->handle, 1);
    ret = libusb_claim_interface(priv->handle, CH347_SPI_IF);
    if (ret < 0) {
        perror("ch347: libusb: claim_if");
        goto ERR_2;
    }

    if (ch347_get_hw_config(priv))
        goto ERR_3;

    return priv;

    ERR_3:
    libusb_release_interface(priv->handle, CH347_SPI_IF);
    ERR_2:
    libusb_close(priv->handle);
    ERR_1:
    libusb_exit(priv->ctx);
    ERR_0:
    free(priv);
    return NULL;
}

void ch347_close(struct ch347_priv *priv) {
    libusb_release_interface(priv->handle, CH347_SPI_IF);
    libusb_close(priv->handle);
    libusb_exit(priv->ctx);
    free(priv);
}


bool ch347_spi_init(uint8_t ch_type, uint8_t i2cBusSpeed) {
    int ret;
    priv = ch347_open();
    if (!priv)  return 1;
    if (ch_type != 1)
    {
        ret = ch347_setup_spi(priv, 3, false, false, false);
        if (ret) return true;
    }

    int freq = 60000; //Speed in KHZ
    switch (ch_type)
          {
             case 0: //SPI NOR FLASH
                freq = 30000;
                break;
             case 1: //24xxx I2C
                freq = 100;

                ch347setI2Cstream(i2cBusSpeed);
                //ch347configure(0x1a86,0x5512);
                //ch347setI2Cstream(priv->handle, 1);
                break;
             case 2: //93xxx Microwire
                freq = 100;
                break;
             case 3: //25xxx
             case 4: //95xxx
             case 5: //45xxx DataFlash
                freq = 5000;
                break;
             case 6:
                freq = 60000;
                break;
             default:
                freq = 60000;
                break;
          }

    fprintf(stderr, "ch347: Clock rate = %d kHZ\n", freq);
    //int freq = 60000;//60000;
    if (ch_type != 1) ret = ch347_set_spi_freq(priv, &freq);
    return ret == 1;
}

void ch347_spi_shutdown()
{
//    if (!priv)
//    {
//        if (ch347i2cClose())
//        return;
//    }
    if (!priv)  return;
    ch347_close(priv);
}
