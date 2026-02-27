//
// ch347eeprom programmer version 0.1 (Beta)
//
//  Programming tool for the 24Cxx serial EEPROMs using the Winchiphead ch347A IC
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

#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include "ch347eeprom.h"
#include "ch347.h"
extern FILE *debugout, *verbout;
extern struct ch347_priv *priv;
struct xxx {
  uint8_t ibuf[512];
  uint8_t obuf[512];
} i2c_dev;

// --------------------------------------------------------------------------
// ch347configure()
//      lock USB device for exclusive use
//      claim default interface
//      set default configuration
//      retrieve device descriptor
//      identify device revision
// returns *usb device handle

//struct libusb_device_handle *priv->handle = NULL;

struct libusb_device_handle *ch347configure(uint16_t vid, uint16_t pid)
{
    int32_t ret = libusb_init(NULL);
    if (ret < 0) {
        fprintf(stderr, "Couldnt initialise libusb\n");
        return NULL;
    }
#if LIBUSBX_API_VERSION < 0x01000106
    libusb_set_debug(NULL, 3);  // maximum debug logging level
#else
    libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
#endif

    fprintf(stderr, "Searching USB buses for WCH ch347a i2c EEPROM programmer [%04x:%04x]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

    //struct libusb_device_handle *priv->handle;
    if (!(priv->handle = libusb_open_device_with_vid_pid(NULL, USB_LOCK_VENDOR, USB_LOCK_PRODUCT)) &&
         !(priv->handle = libusb_open_device_with_vid_pid(NULL, USB_LOCK_VENDOR, USB_LOCK_PRODUCT2))) {
        fprintf(stderr, "Couldn't open device [%04x:{%04x, %04x}]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT, USB_LOCK_PRODUCT2);
        return NULL;
    }

    struct libusb_device *dev;
    if (!(dev = libusb_get_device(priv->handle))) {
        fprintf(stderr, "Couldnt get bus number and address of device\n");
        return NULL;
    }

    fprintf(stderr, "Found [%04x:%04x] as device [%d] on USB bus [%d]\n",
            USB_LOCK_VENDOR, USB_LOCK_PRODUCT, libusb_get_device_address(dev), libusb_get_bus_number(dev));

    fprintf(stderr, "Opened device [%04x:%04x]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

    if (libusb_kernel_driver_active(priv->handle, DEFAULT_INTERFACE)) {
        ret = libusb_detach_kernel_driver(priv->handle, DEFAULT_INTERFACE);
        if (ret) {
            fprintf(stderr, "Failed to detach kernel driver: '%s'\n", strerror(-ret));
            return NULL;
        } else
            fprintf(stderr, "Detached kernel driver\n");
    }

    int32_t currentConfig = 0;
    ret = libusb_get_configuration(priv->handle, &currentConfig);
    if (ret) {
        fprintf(stderr, "Failed to get current device configuration: '%s'\n", strerror(-ret));
        return NULL;
    }

    if (currentConfig != DEFAULT_CONFIGURATION)
        ret = libusb_set_configuration(priv->handle, currentConfig);

    if (ret) {
        fprintf(stderr, "Failed to set device configuration to %d: '%s'\n", DEFAULT_CONFIGURATION, strerror(-ret));
        return NULL;
    }

    ret = libusb_claim_interface(priv->handle, 2); // interface 0

    if (ret) {
        fprintf(stderr, "Failed to claim interface %d: '%s'\n", DEFAULT_INTERFACE, strerror(-ret));
        return NULL;
    }

    fprintf(stderr, "Claimed device interface [%d]\n", 2);

    uint8_t ch347DescriptorBuffer[0x12];
    ret = libusb_get_descriptor(priv->handle, LIBUSB_DT_DEVICE, 0x00, ch347DescriptorBuffer, 0x12);

    if (ret < 0) {
        fprintf(stderr, "Failed to get device descriptor: '%s'\n", strerror(-ret));
        return NULL;
    }

    fprintf(stderr, "Device reported its revision [%d.%02d]\n", ch347DescriptorBuffer[12], ch347DescriptorBuffer[13]);

    for (unsigned i = 0; i < 0x12; ++i)
        fprintf(stderr, "%02x ", ch347DescriptorBuffer[i]);
    fprintf(stderr, "\n");

    return priv->handle;
}

// --------------------------------------------------------------------------
//  ch347setstream()
//      set the i2c bus speed (speed: 0 = 20kHz; 1 = 100kHz, 2 = 400kHz, 3 = 750kHz)
int32_t ch347setstream(uint32_t speed)
{
    int32_t actuallen = 0;

    i2c_dev.obuf[0] = mch347A_CMD_I2C_STREAM;
    i2c_dev.obuf[1] = mch347A_CMD_I2C_STM_SET | (speed & 0x03);
    i2c_dev.obuf[2] = mch347A_CMD_I2C_STM_END;

    int ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, 3, &actuallen, DEFAULT_TIMEOUT);

    if (ret < 0) {
        fprintf(stderr, "ch347setstream(): Failed write %d bytes '%s'\n", 2, strerror(-ret));
        return -1;
    }

    fprintf(stderr, "ch347setstream(): Wrote 2 bytes: ");
    for (unsigned i = 0; i < 2; ++i)
        fprintf(stderr, "%02x ", i2c_dev.obuf[i]);
    fprintf(stderr, "\n");
    return 0;
}


int ch347i2cBlockRead(uint8_t *buf, uint32_t address, uint32_t blockSize, uint8_t algorithm)
{
    int ret;
    int32_t actuallen = 0;
    uint32_t size = blockSize;
    uint8_t *ptr = i2c_dev.obuf;
    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0;

        if (size > 64) size = 64;

        *ptr++ = mch347A_CMD_I2C_STREAM;
        *ptr++ = mch347A_CMD_I2C_STM_STA;

        if ((algorithm & 0x0f) == 0x01) //1 byte address
        {
            *ptr++ = mch347A_CMD_I2C_STM_OUT | 2;
            deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
            wordAddressLo = (uint8_t) (address & 0x00ff);
            *ptr++ = deviceAddress;
            *ptr++ = wordAddressLo;
        }
        if ((algorithm & 0x0f) == 0x02) //2 byte address
        {
            *ptr++ = mch347A_CMD_I2C_STM_OUT | 3;
            deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
            wordAddressLo = (uint8_t) (address & 0x00ff);
            wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
            *ptr++ = deviceAddress;
            *ptr++ = wordAddressHi;
            *ptr++ = wordAddressLo;
        }
            //device addr + read bit
            *ptr++ = mch347A_CMD_I2C_STM_STA;
            *ptr++ = mch347A_CMD_I2C_STM_OUT | 1;
            *ptr++ = deviceAddress | 0x01;

            *ptr++ = mch347A_CMD_I2C_STM_IN | ((uint8_t)(size - 1));

            *ptr++ = mch347A_CMD_I2C_STM_IN; //last byte and end of packet
            *ptr++ = mch347A_CMD_I2C_STM_STO;
            *ptr++ = mch347A_CMD_I2C_STM_END;


        ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, 15 , &actuallen, DEFAULT_TIMEOUT);
        if (ret < 0)
        {
            fprintf(stderr, "USB write error : %s\n", strerror(-ret));
            return ret;
        }
        ret = libusb_bulk_transfer(priv->handle, BULK_READ_ENDPOINT, i2c_dev.ibuf, blockSize + 4, &actuallen, DEFAULT_TIMEOUT);
        if (ret < 0)
        {
            fprintf(stderr, "USB read error : %s\n", strerror(-ret));
            return ret;
        }

        if ((algorithm & 0x0f) == 0x02) memcpy(buf, &i2c_dev.ibuf[4], size);
        else memcpy(buf, &i2c_dev.ibuf[3], size);

    return 0;
}

int ch347i2cBlockWrite(uint8_t *buf, uint32_t address, uint32_t blockSize, uint8_t algorithm)
{
    int ret;
    int32_t actuallen = 0;
    uint32_t size = blockSize;
    uint8_t *ptr = i2c_dev.obuf;
    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0;

        if (size > 64) size = 64;

        *ptr++ = mch347A_CMD_I2C_STREAM;
        *ptr++ = mch347A_CMD_I2C_STM_STA;

        if ((algorithm & 0x0f) == 0x01) //1 byte address
        {
            *ptr++ = mch347A_CMD_I2C_STM_OUT | 2;
            deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
            wordAddressLo = (uint8_t) (address & 0x00ff);
            *ptr++ = deviceAddress;
            *ptr++ = wordAddressLo;
        }
        if ((algorithm & 0x0f) == 0x02) //2 byte address
        {
            *ptr++ = mch347A_CMD_I2C_STM_OUT | 3;
            deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
            wordAddressLo = (uint8_t) (address & 0x00ff);
            wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
            *ptr++ = deviceAddress;
            *ptr++ = wordAddressHi;
            *ptr++ = wordAddressLo;
        }
            *ptr++ = mch347A_CMD_I2C_STM_OUT | ((uint8_t)(size ));

//            for (i = 0; i < size; i++) *ptr++ = buf[i];

             memcpy(ptr, buf, size);
             ptr += size;

            *ptr++ = mch347A_CMD_I2C_STM_STO;
            *ptr++ = mch347A_CMD_I2C_STM_END;


        ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, size + 12 , &actuallen, DEFAULT_TIMEOUT);
        if (ret < 0)
        {
            fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
            return ret;
        }
        ret = libusb_bulk_transfer(priv->handle, BULK_READ_ENDPOINT, i2c_dev.ibuf, 512, &actuallen, DEFAULT_TIMEOUT);

        if (ret < 0) {
            fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
            return -1;
        }

        for (unsigned i = 0; i < actuallen; ++i)
        {
            if (i2c_dev.ibuf[i] != 0x01)
            {
                fprintf(stderr, "received NACK at %d", i);
                return -1;
            }
        }
        ret = ch347delay_ms(10);
        if (ret < 0)
        {
            fprintf(stderr, "Failed to set timeout: '%s'\n", strerror(-ret));
            return -1;
        }

    return 0;
}


int ch347_i2c_read(struct i2c_msg *msg)
{
    unsigned byteoffset = 0;
    while (msg->len - byteoffset > 0) {
        unsigned bytestoread = msg->len - byteoffset;
        if (bytestoread > 63) // reserve first byte for status
            bytestoread = 63;
        uint8_t *ptr = i2c_dev.obuf;
        *ptr++ = mch347A_CMD_I2C_STREAM;
        *ptr++ = mch347A_CMD_I2C_STM_STA;
        *ptr++ = mch347A_CMD_I2C_STM_OUT|1;
        *ptr++ = (msg->addr << 1) | 1;
        if (bytestoread > 1)
            *ptr++ = mch347A_CMD_I2C_STM_IN | (bytestoread - 1);
        *ptr++ = mch347A_CMD_I2C_STM_IN;
        *ptr++ = mch347A_CMD_I2C_STM_STO;
        *ptr++ = mch347A_CMD_I2C_STM_END;

        int actuallen = 0;
        int ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, ptr - i2c_dev.obuf, &actuallen, DEFAULT_TIMEOUT);
        if (ret < 0) {
            fprintf(stderr, "USB write error : %s\n", strerror(-ret));
            return ret;
        }
        ret = libusb_bulk_transfer(priv->handle, BULK_READ_ENDPOINT, i2c_dev.ibuf, 512, &actuallen, DEFAULT_TIMEOUT);
        if (ret < 0) {
            fprintf(stderr, "USB read error : %s\n", strerror(-ret));
            return ret;
        }
        if (actuallen != bytestoread + 1) {
            fprintf(stderr, "actuallen(%d) != bytestoread(%d)\b", actuallen, bytestoread);
            return -1;
        }
        if (i2c_dev.ibuf[0] != 0x01) {
            fprintf(stderr, "received NACK");
            return -1;
        }
        memcpy(&msg->buf[byteoffset], &i2c_dev.ibuf[1], bytestoread);
        byteoffset += bytestoread;
    }
    return 0;
}

int ch347_i2c_write(struct i2c_msg *msg) {
    unsigned left = msg->len;
    uint8_t *ptr = msg->buf;
    bool first = true;
    do {
        uint8_t *outptr = i2c_dev.obuf;
        *outptr++ = mch347A_CMD_I2C_STREAM;
        unsigned wlen = left;
        if (wlen > 62) { // wlen has only 6-bit field in protocol
            wlen = 62;
        }
        if (first) { // Start packet
            *outptr++ = mch347A_CMD_I2C_STM_STA;
            *outptr++ = mch347A_CMD_I2C_STM_OUT | (wlen + 1);
            *outptr++ = msg->addr << 1;
        }
        memcpy(outptr, ptr, wlen);
        outptr += wlen;
        ptr += wlen;
        left -= wlen;

        if (left == 0) {  // Stop packet
            *outptr++ = mch347A_CMD_I2C_STM_STO;
        }
        *outptr++ = mch347A_CMD_I2C_STM_END;
        first = false;
        int actuallen = 0, ret = 0;
        ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, outptr - i2c_dev.obuf, &actuallen, DEFAULT_TIMEOUT);

        if (ret < 0) {
            fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
            return -1;
        }
        ret = libusb_bulk_transfer(priv->handle, BULK_READ_ENDPOINT, i2c_dev.ibuf, 512, &actuallen, DEFAULT_TIMEOUT);

        if (ret < 0) {
            fprintf(stderr, "Failed to write to I2C: '%s'\n", strerror(-ret));
            return -1;
        }
        if (wlen + 1 != actuallen) {
            fprintf(stderr, "failed to get ACKs %d of %d", actuallen, wlen + 1);
            return -1;
        }
        for (unsigned i = 0; i < actuallen; ++i) {
            if (i2c_dev.ibuf[i] != 0x01) {
                fprintf(stderr, "received NACK at %d", i);
                return -1;
            }
        }
    } while (left);

    return 0;
}

int ch347_i2c_xfer(struct i2c_msg *msg, unsigned num) {
    for (unsigned i = 0; i < num; ++i) {
        if (msg[i].flags & I2C_M_RD) {
            int ret = ch347_i2c_read(&msg[i]);
            if (ret) return ret;
        } else {
            int ret = ch347_i2c_write(&msg[i]);
            if (ret) return ret;
        }
    }
    return 0;
}

// --------------------------------------------------------------------------
// ch347readEEPROM()
//      read n bytes from device (in packets of 32 bytes)

//(uint8_t *buffer, uint32_t offset, uint32_t bytestoread, uint32_t ic_size, uint32_t block_size, uint8_t algorithm)
//res = ch341readEEPROM_param(buf.get(), curBlock * step, step, currentChipSize, currentPageSize, currentAlgorithm);//currentAlgorithm);

int ch347readEEPROM(uint8_t *buffer, uint32_t startaddr, uint32_t ic_size, uint16_t blocksize, uint8_t algorithm)
{
    //
        struct EEPROM2 eeprom_info;
        eeprom_info.name = "24c01";
        eeprom_info.size = ic_size;
        eeprom_info.page_size = (uint16_t)blocksize;
        eeprom_info.addr_size = 0x0f & algorithm;
        eeprom_info.i2c_addr_mask = (0xf0 & algorithm) / 16;
        printf("addr size : %x\n", eeprom_info.addr_size);
    //

    struct i2c_msg msg[2];

    uint8_t out[2] = {0};
    uint8_t ext_addr = 0;
    if (eeprom_info.addr_size == 2)
    {
        out[0] = (startaddr & 0xff00) >> 8;
        out[1] = startaddr & 0xff;
    }
    else
    {
        out[0] = startaddr & 0xff;
        //ext_addr = (uint8_t) (0xa0 | (startaddr >> 8 & eeprom_info.i2c_addr_mask) << 1);
        //ext_addr = (uint8_t) (startaddr >> 8) | 0xa0;
        //ext_addr = 0x58 | ((startaddr & 0xff00)>>8);
        ext_addr = (uint8_t) (startaddr >> 8);
        printf("ext addr : %x\n", ext_addr);
    }

    msg[0].len = eeprom_info.addr_size;
    msg[0].flags = ext_addr;//0;
    msg[0].addr = EEPROM_I2C_BUS_ADDRESS;
    msg[0].buf = out;

    msg[1].flags = I2C_M_RD;// | ext_addr;
    msg[1].buf = buffer;
    msg[1].addr = EEPROM_I2C_BUS_ADDRESS;
    msg[1].len = blocksize;

    return ch347_i2c_xfer(msg, 2);
}

int ch347delay_ms(unsigned ms) {
    i2c_dev.obuf[0] = mch347A_CMD_I2C_STREAM;
    i2c_dev.obuf[1] = mch347A_CMD_I2C_STM_MS | (ms & 0xf);        // Wait up to 15ms
    i2c_dev.obuf[2] = mch347A_CMD_I2C_STM_END;
    int actuallen = 0;
    libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, 3, &actuallen, DEFAULT_TIMEOUT);
    return 0;
}

int ch347_quick_write(uint8_t addr) {
    uint8_t * ptr = i2c_dev.obuf;
    *ptr++ = mch347A_CMD_I2C_STREAM;
    *ptr++ = mch347A_CMD_I2C_STM_STA;
    *ptr++ = mch347A_CMD_I2C_STM_OUT;
#if 0
    *ptr++ = addr << 1 | 1; // READ1
    *ptr++ = mch347A_CMD_I2C_STM_IN;
#else
    *ptr++ = addr << 1; // WRITE0
#endif
    *ptr++ = mch347A_CMD_I2C_STM_STO;
    *ptr++ = mch347A_CMD_I2C_STM_END;
    int actuallen = 0;
    int ret = libusb_bulk_transfer(priv->handle, BULK_WRITE_ENDPOINT, i2c_dev.obuf, ptr - i2c_dev.obuf, &actuallen, DEFAULT_TIMEOUT);
    if (ret < 0) {
        fprintf(stderr, "USB write error : %s\n", strerror(-ret));
        return ret;
    }
    ret = libusb_bulk_transfer(priv->handle, BULK_READ_ENDPOINT, i2c_dev.ibuf, 32, &actuallen, DEFAULT_TIMEOUT);
    if (ret < 0) {
        fprintf(stderr, "USB read error : %s\n", strerror(-ret));
        return ret;
    }
    for (unsigned i = 0; i < actuallen; ++i) {
        fprintf(stderr, "%02x ", i2c_dev.ibuf[i]);
    }
    fprintf(stderr, "0x%02x: %s\n", addr, (i2c_dev.ibuf[0] & 0x1) == 0 ? "NACK" : "ACK");
    return 0;
}
// --------------------------------------------------------------------------
// ch347writeEEPROM()
//      write n bytes to 24c32/24c64 device (in packets of 32 bytes)
//int32_t ch347writeEEPROM(uint8_t *buffer, uint32_t bytesum, const struct EEPROM2 eeprom_info)
//{
//    uint8_t msgbuffer[256 + 2]; // max EEPROM page size is 256 in 2M part, and 2 bytes for address
//    struct i2c_msg msg;
//    msg.addr = EEPROM_I2C_BUS_ADDRESS;
//    msg.buf = msgbuffer;
//    msg.flags = 0;
//    unsigned offset = 0;
//    while (offset < bytesum) {
//        uint8_t *outptr = msgbuffer;
//        unsigned wlen = eeprom_info.page_size;
//        if (eeprom_info.addr_size > 1) {
//            *outptr++ = offset >> 8;
//        }
//        *outptr++ = offset;
//        if (bytesum - offset < wlen) {
//            wlen = bytesum - offset;
//        }
//        memcpy(outptr, buffer + offset, wlen);
//        outptr += wlen;
//        msg.len = outptr - msgbuffer;
//        int ret = 0;
//        ret = ch347_i2c_xfer(&msg, 1);
//        if (ret < 0) {
//            fprintf(stderr, "Failed to write to EEPROM: '%s'\n", strerror(-ret));
//            return -1;
//        }
//        offset += wlen;

//        ret = ch347delay_ms(10);
//        if (ret < 0) {
//            fprintf(stderr, "Failed to set timeout: '%s'\n", strerror(-ret));
//            return -1;
//        }
//        fprintf(stdout, "Written %d%% [%d] of [%d] bytes\r", 100 * offset / bytesum, offset, bytesum);
//    }
//    return 0;
//}

// --------------------------------------------------------------------------
// parseEEPsize()
//   passed an EEPROM name (case-sensitive), returns its byte size
//int32_t parseEEPsize(char *eepromname, const struct EEPROM **eeprom)
//{
//    for (unsigned i = 0; eepromlist[i].size; ++i)
//        if (strcmp(eepromlist[i].name, eepromname) == 0) {
//            *eeprom = &eepromlist[i];
//            return (eepromlist[i].size);
//        }
//    return -1;
//}

int ch347i2cOpen()
{
    if (!(priv->handle = ch347configure(USB_LOCK_VENDOR, USB_LOCK_PRODUCT)))
    {
        printf("Couldnt configure USB device with vendor ID: %04x product ID: %04x\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT);
        return 1;

    }
    else return 0;
}

int ch347i2cClose()
{
    libusb_release_interface(priv->handle, DEFAULT_INTERFACE);
        printf("Released device interface [%d]\n", DEFAULT_INTERFACE);
        libusb_close(priv->handle);
        printf("Closed USB device\n");
        libusb_exit(NULL);
        return 0;
}
