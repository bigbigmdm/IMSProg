//  Programming tool for the 24Cxx serial EEPROMs using the Winchiphead ch34x IC
//
// (c) December 2011 asbokid <ballymunboy@gmail.com>
// (c) December 2023 aystarik <aystarik@gmail.com>
// (c) February 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
//
//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "ch34x_i2c.h"
#include "ch347.h"
#include "ch341a_spi.h"
//Initialise and close CH341A see in ch341a_spi.h, CH347 - in ch347.h
extern struct libusb_device_handle *handle;
extern struct ch347_priv *priv;

struct xxx {
  uint8_t ibuf[512];
  uint8_t obuf[512];
} i2c_buf;

uint8_t progDev = 0;

int ch34xdelay_ms(unsigned ms) {
    i2c_buf.obuf[0] = ch34x_CMD_I2C_STREAM;
    i2c_buf.obuf[1] = ch34x_CMD_I2C_STM_MS | (ms & 0xf);        // Wait up to 15ms
    i2c_buf.obuf[2] = ch34x_CMD_I2C_STM_END;
    int actuallen = 0;
    if (progDev > 1) libusb_bulk_transfer(priv->handle, ch347_BULK_WRITE_ENDPOINT, i2c_buf.obuf, 3, &actuallen, DEFAULT_TIMEOUT);
    else libusb_bulk_transfer(handle, ch341_BULK_WRITE_ENDPOINT, i2c_buf.obuf, 3, &actuallen, DEFAULT_TIMEOUT);
    return 0;
}

int ch34xi2cBlockRead(uint8_t *buf, uint32_t address, uint32_t blockSize, uint8_t algorithm, uint8_t progDevice)
{
    int ret;
    uint32_t step, maxstep;
    int32_t actuallen = 0;
    uint32_t size = blockSize;

    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0;
    progDev = progDevice;

        if ((progDev > 1) && (size > 64)) size = 64;
        if ((progDev < 2) && (size > 16)) size = 16;
        maxstep = blockSize / size;

        for (step = 0; step < maxstep; step++)
        {
            uint8_t *ptr = i2c_buf.obuf;
            *ptr++ = ch34x_CMD_I2C_STREAM;
            *ptr++ = ch34x_CMD_I2C_STM_STA;

            if ((algorithm & 0x0f) == 0x01) //1 byte address
            {
                *ptr++ = ch34x_CMD_I2C_STM_OUT | 2;
                deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                *ptr++ = deviceAddress;
                *ptr++ = wordAddressLo;
            }
            if ((algorithm & 0x0f) == 0x02) //2 byte address
            {
                *ptr++ = ch34x_CMD_I2C_STM_OUT | 3;
                deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                *ptr++ = deviceAddress;
                *ptr++ = wordAddressHi;
                *ptr++ = wordAddressLo;
            }
                //device addr + read bit
                *ptr++ = ch34x_CMD_I2C_STM_STA;
                *ptr++ = ch34x_CMD_I2C_STM_OUT | 1;
                *ptr++ = deviceAddress | 0x01;
                *ptr++ = ch34x_CMD_I2C_STM_IN | ((uint8_t)(size - 1));
                *ptr++ = ch34x_CMD_I2C_STM_IN; //last byte and end of packet
                *ptr++ = ch34x_CMD_I2C_STM_STO;
                *ptr++ = ch34x_CMD_I2C_STM_END;


            if (progDev > 1) ret = libusb_bulk_transfer(priv->handle, ch347_BULK_WRITE_ENDPOINT, i2c_buf.obuf, 15 , &actuallen, DEFAULT_TIMEOUT);
            else ret = libusb_bulk_transfer(handle, ch341_BULK_WRITE_ENDPOINT, i2c_buf.obuf, 15 , &actuallen, DEFAULT_TIMEOUT);
            if (ret < 0)
            {
                fprintf(stderr, "USB write error : %s\n", strerror(-ret));
                return ret;
            }
            if (progDev > 1) ret = libusb_bulk_transfer(priv->handle, ch347_BULK_READ_ENDPOINT, i2c_buf.ibuf, blockSize + 4, &actuallen, DEFAULT_TIMEOUT);
            else ret = libusb_bulk_transfer(handle, ch341_BULK_READ_ENDPOINT, i2c_buf.ibuf, blockSize + 4, &actuallen, DEFAULT_TIMEOUT);
            if (ret < 0)
            {
                fprintf(stderr, "USB read error : %s\n", strerror(-ret));
                return ret;
            }

            if (progDev > 1)
            {
                if ((algorithm & 0x0f) == 0x02) memcpy(&buf[step * size], &i2c_buf.ibuf[4], size);
                else memcpy(&buf[step * size], &i2c_buf.ibuf[3], size);
            }
            else memcpy(&buf[step * size], &i2c_buf.ibuf[0], size);

            address = address + size;
            ret = ch34xdelay_ms(10);
            if (ret < 0)
            {
                fprintf(stderr, "Failed to set timeout: '%s'\n", strerror(-ret));
                return -1;
            }
        }
    return 0;
}

int ch34xi2cBlockWrite(uint8_t *buf, uint32_t address, uint32_t blockSize, uint32_t sectorSize, uint8_t algorithm, uint8_t progDevice)
{
    int ret;
    int32_t actuallen = 0;
    uint32_t size = blockSize;
    uint32_t step, maxstep;
    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0;
    progDev = progDevice;

        if (size > sectorSize) size = sectorSize;
        if ((progDev < 2) && (size > 16)) size = 16;
        maxstep = blockSize / size;

        for (step = 0; step < maxstep; step++)
        {
            uint8_t *ptr = i2c_buf.obuf;
            *ptr++ = ch34x_CMD_I2C_STREAM;
            *ptr++ = ch34x_CMD_I2C_STM_STA;

            if ((algorithm & 0x0f) == 0x01) //1 byte address
            {
                *ptr++ = ch34x_CMD_I2C_STM_OUT | 2;
                deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                *ptr++ = deviceAddress;
                *ptr++ = wordAddressLo;
            }
            if ((algorithm & 0x0f) == 0x02) //2 byte address
            {
                *ptr++ = ch34x_CMD_I2C_STM_OUT | 3;
                deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                *ptr++ = deviceAddress;
                *ptr++ = wordAddressHi;
                *ptr++ = wordAddressLo;
            }
                *ptr++ = ch34x_CMD_I2C_STM_OUT | ((uint8_t)(size ));

                 for (uint32_t i = 0; i < size; i++) *ptr++ = buf[i + step * size];

                *ptr++ = ch34x_CMD_I2C_STM_STO;
                *ptr++ = ch34x_CMD_I2C_STM_END;


            if (progDev > 1) ret = libusb_bulk_transfer(priv->handle, ch347_BULK_WRITE_ENDPOINT, i2c_buf.obuf, size + 12 , &actuallen, DEFAULT_TIMEOUT);
            else ret = libusb_bulk_transfer(handle, ch341_BULK_WRITE_ENDPOINT, i2c_buf.obuf, size + 12 , &actuallen, DEFAULT_TIMEOUT);
            if (ret < 0)
            {
                fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
                return ret;
            }
            if (progDev > 1)
            {
                if (progDev > 1) ret = libusb_bulk_transfer(priv->handle, ch347_BULK_READ_ENDPOINT, i2c_buf.ibuf, 512, &actuallen, DEFAULT_TIMEOUT);
                else ret = libusb_bulk_transfer(handle, ch341_BULK_WRITE_ENDPOINT, i2c_buf.obuf, size + 12 , &actuallen, DEFAULT_TIMEOUT);

                if (ret < 0) {
                    fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
                    return -1;
                }

                for (unsigned i = 0; i < actuallen; ++i)
                {
                    if (i2c_buf.ibuf[i] != 0x01)
                    {
                        fprintf(stderr, "received NACK at %d", i);
                        return -1;
                    }
                }

            }
            ret = ch34xdelay_ms(10);
            if (ret < 0)
            {
                fprintf(stderr, "Failed to set timeout: '%s'\n", strerror(-ret));
                return -1;
            }
            address = address + size;
        }

    return 0;
}
