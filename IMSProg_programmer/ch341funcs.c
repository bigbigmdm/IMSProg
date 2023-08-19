//
// ch341eeprom programmer version 0.1 (Beta)
//
//  Programming tool for the 24Cxx serial EEPROMs using the Winchiphead CH341A IC
//
// (c) December 2011 asbokid <ballymunboy@gmail.com>
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

#include <libusb-1.0/libusb.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "ch341funcs.h"

//extern
FILE *debugout, *verbout;
uint32_t getnextpkt;                            // set by the callback function
uint32_t syncackpkt;                            // synch / ack flag used by BULK OUT cb function
uint32_t byteoffset;

// --------------------------------------------------------------------------
// ch341configure()
//      lock USB device for exclusive use
//      claim default interface
//      set default configuration
//      retrieve device descriptor
//      identify device revision
// returns *usb device handle

struct libusb_device_handle *ch341configure(uint16_t vid, uint16_t pid) {

    struct libusb_device *dev;
    struct libusb_device_handle *devHandle;
    int32_t ret=0;                    // set to < 0 to indicate USB errors
    uint32_t i = 0;
    int32_t currentConfig = 0;

    uint8_t ch341DescriptorBuffer[0x12];

    ret = libusb_init(NULL);
    if(ret < 0) {
        fprintf(stderr, "Couldnt initialise libusb\n");
        return NULL;
    }



    #if LIBUSBX_API_VERSION < 0x01000106
        libusb_set_debug(NULL, 3);                  // maximum debug logging level
    #else
        libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
    #endif

    fprintf(verbout, "Searching USB buses for WCH CH341a i2c EEPROM programmer [%04x:%04x]\n",
            USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

    if(!(devHandle = libusb_open_device_with_vid_pid(NULL, USB_LOCK_VENDOR, USB_LOCK_PRODUCT))) {
        fprintf(stderr, "Couldn't open device [%04x:%04x]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT);
        return NULL;
    }

    if(!(dev = libusb_get_device(devHandle))) {
        fprintf(stderr, "Couldnt get bus number and address of device\n");
        return NULL;
    }

    fprintf(verbout, "Found [%04x:%04x] as device [%d] on USB bus [%d]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT,
        libusb_get_device_address(dev), libusb_get_bus_number(dev));

    fprintf(verbout, "Opened device [%04x:%04x]\n", USB_LOCK_VENDOR, USB_LOCK_PRODUCT);


    if(libusb_kernel_driver_active(devHandle, DEFAULT_INTERFACE)) {
        ret = libusb_detach_kernel_driver(devHandle, DEFAULT_INTERFACE);
        if(ret) {
            fprintf(stderr, "Failed to detach kernel driver: '%s'\n", strerror(-ret));
            return NULL;
        } else
            fprintf(verbout, "Detached kernel driver\n");
    }

    ret = libusb_get_configuration(devHandle, &currentConfig);
    if(ret) {
        fprintf(stderr, "Failed to get current device configuration: '%s'\n", strerror(-ret));
        return NULL;
    }

    if(currentConfig != DEFAULT_CONFIGURATION)
        ret = libusb_set_configuration(devHandle, currentConfig);

    if(ret) {
        fprintf(stderr, "Failed to set device configuration to %d: '%s'\n", DEFAULT_CONFIGURATION, strerror(-ret));
        return NULL;
    }

    ret = libusb_claim_interface(devHandle, DEFAULT_INTERFACE); // interface 0

    if(ret) {
        fprintf(stderr, "Failed to claim interface %d: '%s'\n", DEFAULT_INTERFACE, strerror(-ret));
        return NULL;
    }

    fprintf(verbout, "Claimed device interface [%d]\n", DEFAULT_INTERFACE);

    ret = libusb_get_descriptor(devHandle, LIBUSB_DT_DEVICE, 0x00, ch341DescriptorBuffer, 0x12);

    if(ret < 0) {
        fprintf(stderr, "Failed to get device descriptor: '%s'\n", strerror(-ret));
        return NULL;
    }

    fprintf(verbout, "Device reported its revision [%d.%02d]\n",
        ch341DescriptorBuffer[12], ch341DescriptorBuffer[13]);

    for(i=0;i<0x12;i++)
        fprintf(debugout,"%02x ", ch341DescriptorBuffer[i]);
    fprintf(debugout,"\n");

    return devHandle;
}


// --------------------------------------------------------------------------
//  ch341setstream()
//      set the i2c bus speed (speed: 0 = 20kHz; 1 = 100kHz, 2 = 400kHz, 3 = 750kHz)
int32_t ch341setstream(struct libusb_device_handle *devHandle, uint32_t speed) {
    int32_t ret, i;
    uint8_t ch341outBuffer[EEPROM_READ_BULKOUT_BUF_SZ], *outptr;
    int32_t actuallen = 0;

    outptr = ch341outBuffer;

    *outptr++ = mCH341A_CMD_I2C_STREAM;
    *outptr++ = mCH341A_CMD_I2C_STM_SET | (speed & 0x3);
    *outptr   = mCH341A_CMD_I2C_STM_END;

    ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer, 3, &actuallen, DEFAULT_TIMEOUT);

    if(ret < 0) {
      fprintf(stderr, "ch341setstream(): Failed write %d bytes '%s'\n", 3, strerror(-ret));
      return -1;
    }

    fprintf(debugout, "ch341setstream(): Wrote %d bytes: ", 3);
    for(i=0; i < 3; i++)
        fprintf(debugout, "%02x ", ch341outBuffer[i]);
    fprintf(debugout, "\n");
    return 0;
}

size_t ch341ReadCmdMarshall(uint8_t *buffer, uint32_t addr, struct EEPROM *eeprom_info) {
    uint8_t *ptr = buffer;

    // Frame 1. Program data address, and read 32 bytes.
    *ptr++ = mCH341A_CMD_I2C_STREAM; // 0
    *ptr++ = mCH341A_CMD_I2C_STM_STA; // 1
    // Write address
    *ptr++ = mCH341A_CMD_I2C_STM_OUT | ((*eeprom_info).addr_size+1); // 2: I2C bus adddress + EEPROM address
    uint8_t msb_addr;
    if ((*eeprom_info).addr_size >= 2) {
        // 24C32 and more
        msb_addr = addr>>16 & (*eeprom_info).i2c_addr_mask;
        *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1; // 3
        *ptr++ = (addr>>8 & 0xFF); // 4
        *ptr++ = (addr>>0 & 0xFF); // 5
    } else {
        // 24C16 and less
        msb_addr = addr>>8 & (*eeprom_info).i2c_addr_mask;
        *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1; // 3
        *ptr++ = (addr>>0 & 0xFF); // 4
    }
    *ptr++ = mCH341A_CMD_I2C_STM_STA; // 6/5
    *ptr++ = mCH341A_CMD_I2C_STM_OUT | 1; // 7/6
    *ptr++ = (EEPROM_I2C_BUS_ADDRESS | msb_addr)<<1 | 1; // 8/7: Read command

    // Read 32 bytes
    *ptr++ = mCH341A_CMD_I2C_STM_IN | 32; // 9/8
    *ptr++ = mCH341A_CMD_I2C_STM_END; // 10/9

    // Frame 2 - read 32 bytes
    ptr = &buffer[32];
    *ptr++ = mCH341A_CMD_I2C_STREAM;
    *ptr++ = mCH341A_CMD_I2C_STM_IN | 32;
    *ptr++ = mCH341A_CMD_I2C_STM_END;

    // Frame 3 - read 32 bytes
    ptr = &buffer[64];
    *ptr++ = mCH341A_CMD_I2C_STREAM;
    *ptr++ = mCH341A_CMD_I2C_STM_IN | 32;
    *ptr++ = mCH341A_CMD_I2C_STM_END;

    // Finalize - read last 32 bytes
    ptr = &buffer[96];
    *ptr++ = mCH341A_CMD_I2C_STREAM;
    *ptr++ = mCH341A_CMD_I2C_STM_IN | 31;
    *ptr++ = mCH341A_CMD_I2C_STM_IN;
    *ptr++ = mCH341A_CMD_I2C_STM_STO;
    *ptr++ = mCH341A_CMD_I2C_STM_END;

    return ptr - buffer;
}

// --------------------------------------------------------------------------
// ch341readEEPROM()
//      read n bytes from device (in packets of 32 bytes)
int32_t ch341readEEPROM(struct libusb_device_handle *devHandle, uint8_t *buffer, uint32_t bytestoread, struct EEPROM *eeprom_info) {

    uint8_t ch341outBuffer[EEPROM_READ_BULKOUT_BUF_SZ];
    uint8_t ch341inBuffer[IN_BUF_SZ];               // 0x100 bytes
    int32_t ret = 0, readpktcount = 0;
    struct libusb_transfer *xferBulkIn, *xferBulkOut;
    struct timeval tv = {0, 100};                   // our async polling interval
    size_t xfer_size;

    xferBulkIn  = libusb_alloc_transfer(0);
    xferBulkOut = libusb_alloc_transfer(0);

    if(!xferBulkIn || !xferBulkOut) {
        fprintf(stderr, "Couldnt allocate USB transfer structures\n");
        return -1;
    }

    byteoffset = 0;

    fprintf(debugout, "Allocated USB transfer structures\n");

    memset(ch341inBuffer, 0, EEPROM_READ_BULKIN_BUF_SZ);
    xfer_size = ch341ReadCmdMarshall(ch341outBuffer, 0, eeprom_info); // Fill output buffer

    libusb_fill_bulk_transfer(xferBulkIn,  devHandle, BULK_READ_ENDPOINT, ch341inBuffer,
        EEPROM_READ_BULKIN_BUF_SZ, cbBulkIn, NULL, DEFAULT_TIMEOUT);

    libusb_fill_bulk_transfer(xferBulkOut, devHandle, BULK_WRITE_ENDPOINT,
        ch341outBuffer, xfer_size, cbBulkOut, NULL, DEFAULT_TIMEOUT);

    fprintf(debugout, "Filled USB transfer structures\n");

    libusb_submit_transfer(xferBulkIn);
    fprintf(debugout, "Submitted BULK IN start packet\n");
    libusb_submit_transfer(xferBulkOut);
    fprintf(debugout, "Submitted BULK OUT setup packet\n");

    readbuf = buffer;

    while (1) {
        fprintf(stdout, "Read %d%% [%d] of [%d] bytes      \r", 100*byteoffset/bytestoread, byteoffset, bytestoread);
		ret = libusb_handle_events_timeout(NULL, &tv);

		if (ret < 0 || getnextpkt == -1) {          // indicates an error
            fprintf(stderr, "ret from libusb_handle_timeout = %d\n", ret);
            fprintf(stderr, "getnextpkt = %d\n", getnextpkt);
            if (ret < 0)
                fprintf(stderr, "USB read error : %s\n", strerror(-ret));
            libusb_free_transfer(xferBulkIn);
            libusb_free_transfer(xferBulkOut);
            return -1;
        }
        if(getnextpkt == 1) {                       // callback function reports a new BULK IN packet received
            getnextpkt = 0;                         //   reset the flag
            readpktcount++;                         //   increment the read packet counter
            byteoffset += EEPROM_READ_BULKIN_BUF_SZ;
            if (byteoffset == bytestoread)
                break;

            fprintf(debugout, "\nRe-submitting transfer request to BULK IN endpoint\n");
            libusb_submit_transfer(xferBulkIn);     // re-submit request for next BULK IN packet of EEPROM data
            if(syncackpkt)
                syncackpkt = 0;
                                                    // if 4th packet received, we are at end of 0x80 byte data block,
                                                    // if it is not the last block, then resubmit request for data
            if(readpktcount==4) {
                fprintf(debugout, "\nSubmitting next transfer request to BULK OUT endpoint\n");
                readpktcount = 0;

                ch341ReadCmdMarshall(ch341outBuffer, byteoffset, eeprom_info); // Fill output buffer
                libusb_fill_bulk_transfer(xferBulkOut, devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer,
                                    EEPROM_READ_BULKOUT_BUF_SZ, cbBulkOut, NULL, DEFAULT_TIMEOUT);

                libusb_submit_transfer(xferBulkOut);// update transfer struct (with new EEPROM page offset)
                                                    // and re-submit next transfer request to BULK OUT endpoint
            }
        }
	}

    libusb_free_transfer(xferBulkIn);
    libusb_free_transfer(xferBulkOut);
    return 0;
}

// Callback function for async bulk in comms
void cbBulkIn(struct libusb_transfer *transfer) {
    int i;

    switch(transfer->status) {
        case LIBUSB_TRANSFER_COMPLETED:
                                                    // display the contents of the BULK IN data buffer
            fprintf(debugout,"\ncbBulkIn(): status %d - Read %d bytes\n",transfer->status,transfer->actual_length);

            for(i=0; i < transfer->actual_length; i++) {
                if(!(i%16))
                    fprintf(debugout, "\n   ");
                fprintf(debugout, "%02x ", transfer->buffer[i]);
            }
            fprintf(debugout, "\n");
                                                    // copy read data to our EEPROM buffer
            memcpy(readbuf + byteoffset, transfer->buffer, transfer->actual_length);
            getnextpkt = 1;
            break;
        default:
            fprintf(stderr, "\ncbBulkIn: error : %d\n", transfer->status);
            getnextpkt = -1;
    }
	return;
}

// Callback function for async bulk out comms
void cbBulkOut(struct libusb_transfer *transfer) {
    syncackpkt = 1;
    fprintf(debugout, "\ncbBulkOut(): Sync/Ack received: status %d\n", transfer->status);
    return;
}

// --------------------------------------------------------------------------
// ch341writeEEPROM()
//      write n bytes to 24c32/24c64 device (in packets of 32 bytes)
int32_t ch341writeEEPROM(struct libusb_device_handle *devHandle, uint8_t *buffer, uint32_t bytesum, struct EEPROM *eeprom_info) {

    uint8_t ch341outBuffer[512/*EEPROM_WRITE_BUF_SZ*/];
    uint8_t *outptr, *bufptr;
    uint8_t i2cCmdBuffer[256];
    int32_t ret = 0, i;
    uint32_t byteoffset = 0;
    uint32_t bytes = bytesum;
    uint8_t addrbytecount = (*eeprom_info).addr_size+1;  // 24c32 and 24c64 (and other 24c??) use 3 bytes for addressing
    int32_t actuallen = 0;
    uint16_t page_size = (*eeprom_info).page_size;

    bufptr = buffer;

    while(bytes) {
        outptr = i2cCmdBuffer;
        if ((*eeprom_info).addr_size >= 2) {
            *outptr++ = (uint8_t) (0xa0 | (byteoffset >> 16 & (*eeprom_info).i2c_addr_mask)<<1);  // EEPROM device address
            *outptr++ = (uint8_t) (byteoffset >> 8 & 0xff);     // MSB (big-endian) byte address
        } else {
            *outptr++ = (uint8_t) (0xa0 | (byteoffset >> 8 & (*eeprom_info).i2c_addr_mask)<<1);  // EEPROM device address
        }
        *outptr++ = (uint8_t) (byteoffset & 0xff);          // LSB of 16-bit    byte address

        memcpy(outptr, bufptr, page_size); // Copy one page

        byteoffset += page_size;
        bufptr     += page_size;
        bytes      -= page_size;

        outptr    = ch341outBuffer;
        uint16_t page_size_left = page_size + addrbytecount;
        uint8_t part_no = 0;
        uint8_t *i2cBufPtr = i2cCmdBuffer;
        while(page_size_left) {
            uint8_t to_write = MIN(page_size_left, 28);
            *outptr++ = mCH341A_CMD_I2C_STREAM;
            if (part_no == 0) { // Start packet
                *outptr++ = mCH341A_CMD_I2C_STM_STA;
            }
            *outptr++ = mCH341A_CMD_I2C_STM_OUT | to_write;
            memcpy(outptr, i2cBufPtr, to_write);
            outptr += to_write;
            i2cBufPtr += to_write;
            page_size_left -= to_write;

            if (page_size_left == 0) { // Stop packet
                *outptr++ = mCH341A_CMD_I2C_STM_STO;
            }
            *outptr++ = mCH341A_CMD_I2C_STM_END;
            part_no++;
        }
        uint32_t payload_size = outptr - ch341outBuffer;

        for(i=0; i < payload_size; i++) {
            if(!(i%0x10))
                fprintf(debugout, "\n%04x : ", i);
            fprintf(debugout, "%02x ", ch341outBuffer[i]);
        }
        fprintf(debugout, "\n");

        ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT,
            ch341outBuffer, payload_size, &actuallen, DEFAULT_TIMEOUT);

        if(ret < 0) {
            fprintf(stderr, "Failed to write to EEPROM: '%s'\n", strerror(-ret));
            return -1;
        }

        fprintf(debugout, "Writing [aa 5a 00] to EEPROM\n");   // Magic CH341a packet! Undocumented, unknown purpose

        outptr    = ch341outBuffer;
        *outptr++ = mCH341A_CMD_I2C_STREAM;
        *outptr++ = mCH341A_CMD_I2C_STM_MS | 10; // Wait 10ms (?)
        *outptr++ = mCH341A_CMD_I2C_STM_END;

        ret = libusb_bulk_transfer(devHandle, BULK_WRITE_ENDPOINT, ch341outBuffer, 3, &actuallen, DEFAULT_TIMEOUT);

        if(ret < 0) {
            fprintf(stderr, "Failed to write to EEPROM: '%s'\n", strerror(-ret));
            return -1;
        }

        /*
        struct timeval tv = {0, 100};                   // our async polling interval
		ret = libusb_handle_events_timeout(NULL, &tv);
		if (ret < 0) {          // indicates an error
            fprintf(stderr, "ret from libusb_handle_timeout = %d\n", ret);
            fprintf(stderr, "USB read error : %s\n", strerror(-ret));
            return -1;
        }
        */
        fprintf(stdout, "Written %d%% [%d] of [%d] bytes      \r", 100*(bytesum-bytes)/bytesum, bytesum-bytes, bytesum);
    }
    return 0;
}


// --------------------------------------------------------------------------
// parseEEPsize()
//   passed an EEPROM name (case-sensitive), returns its byte size
int32_t parseEEPsize(char* eepromname, struct EEPROM *eeprom) {
    int i;

    for(i=0; eepromlist[i].size; i++)
        if(strstr(eepromlist[i].name, eepromname)) {
            memcpy(eeprom, &(eepromlist[i]), sizeof(struct EEPROM));
            return(eepromlist[i].size);
        }
    return -1;
}
