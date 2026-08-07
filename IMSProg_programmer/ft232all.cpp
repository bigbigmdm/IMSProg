/*
 * Copyright (C) 2026 Mikhail Medvedev <e-ink-reader@yandex.ru>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// Compile: g++ ft232all.cpp -o ft232all -lftdi

#include <ftdi.h>
#include <usb.h>
#include <stdio.h>
#include <stdint.h>  
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "ft232all.h"
#include <libusb.h>

#define VENDOR  0x0403
#define PRODUCT 0x6014

//using namespace Ftdi;

namespace Pin {
    enum bus_t {
       // I2C
       SCL = 0x01,     // ADBUS0 (SCL clock)
       SDA_OUT = 0x02, // ADBUS1 (SDA out)
       SDA_IN = 0x04,  // ADBUS2 (SDA in)
       // SPI, MicroWire
       SK = 0x01, // ADBUS0, SPI data clock
       DO = 0x02, // ADBUS1, SPI data out
       DI = 0x04, // ADBUS2, SPI data in
       CS = 0x08, // ADBUS3, SPI chip select
       
       L0 = 0x10, // ADBUS4, general-ourpose i/o, GPIOL0
       L1 = 0x20, // ADBUS5, general-ourpose i/o, GPIOL1
       L2 = 0x40, // ADBUS6, general-ourpose i/o, GPIOL2
       L3 = 0x80  // ADBUS7, general-ourpose i/o, GPIOL3
    };
}

// Default pin directions I2C (1 = Output, 0 = Input)
const unsigned char dirWrite = Pin::SCL | Pin::SDA_OUT;
const unsigned char dirRead  = Pin::SCL; 
// SPI
// Set these pins high
const unsigned char pinInitialState = Pin::CS|Pin::L0|Pin::L1;
// Use these pins as outputs
const unsigned char pinDirection    = Pin::SK|Pin::DO|Pin::CS|Pin::L0|Pin::L1;

static uint8_t swap_byte(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}
struct ftdi_context ftdi;
struct libusb_device *dev;
struct libusb_device_descriptor desc;

int initFt232h(void)
{

    int ftdi_status = 0;
    ftdi_status = ftdi_init(&ftdi);
    if ( ftdi_status != 0 ) {
        std::cout << "Failed to initialize device\n";
        return -1;
    }
    ftdi_status = ftdi_usb_open(&ftdi, VENDOR, PRODUCT);
    if ( ftdi_status != 0 )
    {
        std::cout << "Can't open device. Got error\n"
                  << ftdi_get_error_string(&ftdi) << '\n';
        return -1;
    }
    ftdi_usb_reset(&ftdi);
    ftdi_set_interface(&ftdi, INTERFACE_ANY);
    ftdi_set_bitmode(&ftdi, 0, 0); // reset
    ftdi_set_bitmode(&ftdi, 0, BITMODE_MPSSE); // Enable MPSSE mode
    ftdi_tcioflush(&ftdi);
    usleep(50000); // sleep 50 ms for setup to complete
    return 0;
}

void closeFt232h(void)
{
    // Проверяем, существует ли активный USB-хэндл
    if (ftdi.usb_dev != NULL) {
        ftdi_usb_reset(&ftdi);
        ftdi_usb_close(&ftdi);
    }

    //ftdi_deinit(&ftdi);
}

int ft232hGetDescriptor(uint8_t *buf)
{
    int ret = 0;
    dev = libusb_get_device(ftdi.usb_dev);
    ret = libusb_get_device_descriptor(dev, &desc);
    if(ret < 0) printf("Failed to get device descriptor: '%x'\n", ret);
    else memcpy(buf, &desc, 0x12);
    return ret;
}

//---------------------------I2C-----------------------------------------
int ft232hSetSpeedI2C(uint16_t speed_khz)
{
    unsigned char buf[16] = {0};
    uint8_t ptr = 0;
    
    if (speed_khz <  20)    speed_khz = 20;
    if (speed_khz > 1000) speed_khz = 1000;  
    
    uint16_t divisor = (20000 / speed_khz) - 1; 

    buf[ptr++] = 0x8A; 
    buf[ptr++] = 0x97; 
    buf[ptr++] = 0x8C; 
    
    buf[ptr++] = 0x9E; 
    buf[ptr++] = 0x07; 
    buf[ptr++] = 0x00; 
    
    buf[ptr++] = 0x85; 

    buf[ptr++] = 0x86;
    buf[ptr++] = divisor & 0xFF;  
    buf[ptr++] = divisor >> 8;    

    if (ftdi_write_data(&ftdi, buf, ptr) != ptr) {
       std::cout << "Set Speed failed\n";
       return -1;
    }
    return 0;
}

static void SetI2CLinesIdle(unsigned char *buf, uint32_t *ptr)
{
    buf[(*ptr)++] = 0x80; 
    buf[(*ptr)++] = Pin::SCL | Pin::SDA_OUT; 
    buf[(*ptr)++] = dirWrite;
}

// Generate I2C Start Condition and turn on the LED on AC6 (low level)
static void SetI2CStart(unsigned char *buf, uint32_t *ptr)
{
    // Turn the LED on (AC6 is set to 0, direction as output)
    buf[(*ptr)++] = 0x82; // Command to write to ACbus
    buf[(*ptr)++] = 0x00; // Data level (bits: AC6 = 0)
    buf[(*ptr)++] = 0x40; // Direction (bit 6 configured as output)

    // Step 1: Hold SCL at 1, pull SDA down to 0
    for (int i = 0; i < 64; i++) {
        buf[(*ptr)++] = 0x80; 
        buf[(*ptr)++] = Pin::SCL; 
        buf[(*ptr)++] = dirWrite;
    }
    // Step 2: Pull SCL down to 0
    for (int i = 0; i < 64; i++) {
        buf[(*ptr)++] = 0x80; 
        buf[(*ptr)++] = 0x00; 
        buf[(*ptr)++] = dirWrite;
    }
}

// Generate I2C Stop Condition and turn off the LED on AC6 (high level)
static void SetI2CStop(unsigned char *buf, uint32_t *ptr)
{
    // Step 1: Hold SCL at 0, SDA at 0
    for (int i = 0; i < 63; i++) {
        buf[(*ptr)++] = 0x80; 
        buf[(*ptr)++] = 0x00; 
        buf[(*ptr)++] = dirWrite;
    }
    // Step 2: Bring SCL up to 1
    for (int i = 0; i < 63; i++) {
        buf[(*ptr)++] = 0x80; 
        buf[(*ptr)++] = Pin::SCL; 
        buf[(*ptr)++] = dirWrite;
    }
    // Step 3: Release SDA to 1 while SCL is high -> STOP
    for (int i = 0; i < 63; i++) {
        buf[(*ptr)++] = 0x80; 
        buf[(*ptr)++] = Pin::SCL | Pin::SDA_OUT; 
        buf[(*ptr)++] = dirWrite;
    }

    // Turn the LED off (AC6 is set to 1, direction as output)
    buf[(*ptr)++] = 0x82; // Command to write to ACbus
    buf[(*ptr)++] = 0x40; // Data level (bits: AC6 = 1)
    buf[(*ptr)++] = 0x40; // Direction (bit 6 configured as output)
}

static void SendByteAndCheckACK(unsigned char *buf, uint32_t *ptr, uint8_t data)
{
    buf[(*ptr)++] = 0x80; 
    buf[(*ptr)++] = 0x00; 
    buf[(*ptr)++] = dirWrite;

    buf[(*ptr)++] = MPSSE_DO_WRITE | MPSSE_WRITE_NEG; 
    buf[(*ptr)++] = 0x00; 
    buf[(*ptr)++] = 0x00; 
    buf[(*ptr)++] = data;

    buf[(*ptr)++] = 0x80; 
    buf[(*ptr)++] = 0x00; 
    buf[(*ptr)++] = dirRead;

    buf[(*ptr)++] = 0x22; 
    buf[(*ptr)++] = 0x00; 
}

int ft232I2cBlockWrite(uint8_t *data, uint32_t address, uint32_t blockSize, uint32_t sectorSize, uint8_t algorithm)
{
    unsigned char buf[1024] = {0};
    uint32_t ptr, i;

    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0; //for 24c32 and more
    
    int ret;
    uint32_t step, maxstep;
    int32_t actuallen = 0;
    uint32_t size = blockSize;
    
    if (size > sectorSize) size = sectorSize;
    maxstep = blockSize / size;

    for (step = 0; step < maxstep; step++)
    {
        ptr = 0;
        // Set lines to idle state
        SetI2CLinesIdle(buf, &ptr);
        // Start Condition
        SetI2CStart(buf, &ptr);
        
            if ((algorithm & 0x0f) == 0x01) //1 byte address
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x02) //2 byte address
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x0a) //2 byte address, swap bits for 24LC1025
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0x010000) >> 14) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x0e) //2 byte address, move high bit and swap bits for 24LC515
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0x008000) >> 13) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }

            for (i = 0; i < size; i++) 
            {
                SendByteAndCheckACK(buf, &ptr, data[i + step * size]);
            }                    

        // Transaction termination (Stop Condition)
        SetI2CStop(buf, &ptr);
    
        buf[ptr++] = SEND_IMMEDIATE;

        if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
        {
            return -1;
        }

        int expected_acks = size + 1;
        unsigned char rx_buf[1024] = {0};
        int total_read = 0;
        int timeout = 0;
    
        while (total_read < expected_acks && timeout < 100)
        {
            int read_bytes = ftdi_read_data(&ftdi, rx_buf + total_read, expected_acks - total_read);
            if (read_bytes > 0) total_read += read_bytes;
            timeout++;
            usleep(5000);
        }

        if (total_read < expected_acks) return -1;

        for (int j = 0; j < total_read; j++) 
        {
            if (rx_buf[j] & 0x01) return -1;         
        }
        address = address + size;
        ftdi_tcioflush(&ftdi);
    }
    return 0;
}

int ft232I2cBlockRead(uint8_t *data, uint32_t address, uint32_t blockSize, uint8_t algorithm)
{
    unsigned char buf[4096] = {0};
    uint32_t ptr, i;

    uint8_t deviceAddress = 0;
    uint8_t wordAddressLo = 0;
    uint8_t wordAddressHi = 0;
    
    int ret;
    uint32_t step, maxstep;
    int32_t actuallen = 0;
    uint32_t size = blockSize;
    
    //if (size > 64) size = 64;
    if (size > 8) size = 8;
    maxstep = blockSize / size;

    for (step = 0; step < maxstep; step++)
    {
        ptr = 0;
        // Set lines to idle state
        SetI2CLinesIdle(buf, &ptr);
        //First Start Condition
        SetI2CStart(buf, &ptr);
        
            if ((algorithm & 0x0f) == 0x01) //1 byte address
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0xff00) >> 8) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x02) //2 byte address
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0xff0000) >> 16) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x0a) //2 byte address, swap bits for 24LC1025
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0x010000) >> 14) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            if ((algorithm & 0x0f) == 0x0e) //2 byte address, move high bit and swap bits for 24LC515
            {
                
                deviceAddress = (uint8_t) ( ((((address & 0x008000) >> 13) & ((algorithm & 0xf0) >> 4)) << 1) | 0xa0);
                wordAddressLo = (uint8_t) (address & 0x00ff);
                wordAddressHi = (uint8_t) ((address & 0xff00) >> 8);
                SendByteAndCheckACK(buf, &ptr, deviceAddress);
                SendByteAndCheckACK(buf, &ptr, wordAddressHi);
                SendByteAndCheckACK(buf, &ptr, wordAddressLo);
            }
            // Set lines to idle state
            SetI2CLinesIdle(buf, &ptr);
            // Repeated Start Condition — key moment for reading!
            SetI2CStart(buf, &ptr);
            // Send base device address with read bit (R/W = 1)
            SendByteAndCheckACK(buf, &ptr, (deviceAddress | 0x01));

            for (i = 0; i < size; i++) 
            {
                // Switch SDA to High-Z (input) to receive data from the chip
                buf[ptr++] = 0x80; 
                buf[ptr++] = 0x00; 
                buf[ptr++] = dirRead;
                // Hardware read of 1 byte (0x20 — read bytes on rising edge, MSB first)
                buf[ptr++] = 0x20; 
                buf[ptr++] = 0x00; 
                buf[ptr++] = 0x00; // Length 0 -> 1 byte
                
                bool is_last = (i == size - 1);
                
                // Return SDA to output to send ACK (0) or NACK (1)
                buf[ptr++] = 0x80; 
                buf[ptr++] = is_last ? Pin::SDA_OUT : 0x00; // NACK (1) on the last byte, ACK (0) on others
                buf[ptr++] = dirWrite;
                
                // Transmit 1 bit of ACK/NACK
                buf[ptr++] = 0x13; 
                buf[ptr++] = 0x00; 
                buf[ptr++] = is_last ? 0x80 : 0x00; 
            }                    
        // 7. Transaction termination (Stop Condition)
        SetI2CStop(buf, &ptr);
    
        buf[ptr++] = SEND_IMMEDIATE;

        if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
        {
            return -1;
        }

        // Wait for responses: 3 ACKs (write address + memory address + read address) + len bytes of data
        int expected_acks = 3; 
        int total_expected = expected_acks + size;
        unsigned char rx_buf[2048] = {0};
        int total_read = 0;
        int timeout = 0;
    
        while (total_read < total_expected && timeout < 100) 
        {
            int read_bytes;
            if ((algorithm & 0x0f) == 0x01) read_bytes = ftdi_read_data(&ftdi, rx_buf + total_read, total_expected - total_read);
            else read_bytes = ftdi_read_data(&ftdi, rx_buf + total_read -1, total_expected - total_read +1);
            if (read_bytes > 0) total_read += read_bytes;
            timeout++;
            usleep(100);
        }

        if (total_read < expected_acks) return -1;

        // Extract useful data that come right after all configuration ACK bits
        for (uint16_t j = 0; j < size; j++) 
        {
            data[j + step * size] = rx_buf[expected_acks + j];
        }
        address = address + size;
        ftdi_tcioflush(&ftdi);
    }
    return 0;
}

//---------------------------SPI-----------------------------------------
// SPI commands

int ft232h_CS_LO(void)
{
   unsigned char buf[8] = {0};
   uint8_t ptr = 0;
   // Next three commands sets the GPIOL0 pin low. Pulling CS low.
   buf[ptr++] = 0x80;
   buf[ptr++] = 0x00;
   buf[ptr++] = 0x1b;
   ftdi_tcoflush(&ftdi);
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write failed\n";
      return -1;
   }
   else return 0;
}

int ft232h_CS_HI(void)
{
   unsigned char buf[8] = {0};
   uint8_t ptr = 0;
   // Next three commands sets the GPIOL0 pin high. Pulling CS high.
   buf[ptr++] = 0x80;
   buf[ptr++] = 0x08;
   buf[ptr++] = 0x1b;
   buf[ptr++] = SEND_IMMEDIATE;
   ftdi_tcoflush(&ftdi);
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write failed\n";
      return -1;
   }
   else return 0;
}

int ft232hSetSpeedSPI(uint16_t speed_khz)
{
   unsigned char buf[16] = {0};
   uint8_t ptr = 0;
   
   // 1. Constraint frequency to the FT232H limits (in kHz)
   if (speed_khz < 100) speed_khz = 100;
   if (speed_khz > 30000) speed_khz = 30000; // Hardware limit is 30 MHz!
   
   // 2. Calculate the divisor
   // Formula: TCK = 60000 kHz / ((1 + Divisor) * 2)
   // Derived: Divisor = (30000 / TCK) - 1
   uint16_t divisor = (30000 / speed_khz) - 1;

   // 3. Assemble the command packet
   // DIS_DIV_5 must be active to set the base clock to 60 MHz instead of 12 MHz
   buf[ptr++] = DIS_DIV_5;       // opcode (0x8A): Disable division by 5 
   
   buf[ptr++] = TCK_DIVISOR;     // opcode (0x86): set clk divisor
   buf[ptr++] = divisor & 0xFF;  // argument: low byte of divisor
   buf[ptr++] = divisor >> 8;    // argument: high byte of divisor
   
   buf[ptr++] = LOOPBACK_END;    // opcode (0x85): disable loopback
   
   buf[ptr++] = 0x80;            // opcode (0x80): set low bits (ADBUS[0-7])
   buf[ptr++] = 0x00;            // argument: initial pin states
   buf[ptr++] = 0x0B;            // argument: pin direction (1 = output, 0 = input)

   // 4. Send packet to FTDI
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write failed\n";
      return -1;
   }
   
   return 0;
}

int ft232WriteNbytes(uint8_t *buffer, uint32_t sizeToWrite)
{
   unsigned char buf[32768] = {0};
   uint8_t ptr = 0;
   uint32_t i;
   
   sizeToWrite--;
   buf[ptr++] = MPSSE_DO_WRITE | MPSSE_WRITE_NEG; //0x11
   buf[ptr++] = sizeToWrite & 0xff;
   buf[ptr++] = sizeToWrite >> 8;
   for (i = 0; i <= sizeToWrite; i++) buf[ptr++] = buffer[i];
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write failed\n";
      return -1;
   }
   else return 0;
}

int ft232ReadNbytes(uint8_t *buffer, uint32_t sizeToRead)
{
   unsigned char buf[131072] = {0};
   uint8_t ptr = 0;
   uint32_t i;
   sizeToRead--;
   buf[ptr++] = 0x20; // MPSSE_DO_READ (или 0x24 с READ_NEG, проверьте что стабильнее)
   buf[ptr++] = sizeToRead & 0xFF;
   buf[ptr++] = sizeToRead >> 8;
   buf[ptr++] = 0x87;//SEND_IMMEDIATE;
   
   
   //for (i = 0; i <= sizeToWrite; i++) buf[ptr++] = buffer[i];
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write reading size failed\n";
      return -1;
   }
   
    // Reading FTDI buffer
    int bytes_read = 0;
    int total_read = 0;
    while (total_read < sizeToRead) {
        bytes_read = ftdi_read_data(&ftdi, buffer + total_read, sizeToRead - total_read);
        if (bytes_read < 0) break;
        total_read += bytes_read;
    }

   ftdi_tcioflush(&ftdi);
   return 0;
}


//------------------------MicroWire-----------------------------------------
int initFt232hMW(void)
{
    int ftdi_status = 0;
    ftdi_status = ftdi_init(&ftdi);
    if (ftdi_status != 0) {
       std::cout << "Failed to initialize FT232H device\n";
       return -1;
    }
    
    ftdi_status = ftdi_usb_open(&ftdi, VENDOR, PRODUCT);
    if (ftdi_status != 0) {
       std::cout << "Can't open FT232H device. \n"
                 << ftdi_get_error_string(&ftdi) << '\n';
       return -1;
    }
    
    ftdi_usb_reset(&ftdi);
    ftdi_set_interface(&ftdi, INTERFACE_ANY);
    
    ftdi_set_bitmode(&ftdi, 0, 0x00); 
    ftdi_set_bitmode(&ftdi, 0, 0x02); // Enable MPSSE mode
    ftdi_tcioflush(&ftdi);
    usleep(50000);
    
   unsigned char buf[8] = {0};
   uint8_t ptr = 0;
   // Next three commands sets the GPIOL0 pin low. Pulling CS low.
   buf[ptr++] = 0x80;
   buf[ptr++] = 0x00;
   buf[ptr++] = 0x1b;
   ftdi_tcoflush(&ftdi);
   if ( ftdi_write_data(&ftdi, buf, ptr) != ptr ) 
   {
      std::cout << "Write failed\n";
      return -1;
   }
   ftdi_tcioflush(&ftdi);
   usleep(50000);
      
    return 0;
}

static int ft232hMWWaitForReady()
{
    //Wait for ready status
    uint8_t buf[2048];
    int ptr, j;
    uint8_t gpio_val = 0;
    usleep(50);
    ptr = 0;
    j = 0;
    for (j = 0; j < 8; j++)
    {
        buf[ptr++] = 0x80;
        buf[ptr++] =  Pin::CS; // CS High
        buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
    }

    buf[ptr++] = 0x87; // SEND_IMMEDIATE;
    if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
    {
        std::cout << "Write reading size failed\n";
        return -1;
    }


    for (j = 0; j < 255; j++)
    {
        // MPSSE 0x81: Read Low Byte (GPIOL0..7)
        ptr = 0;
        buf[ptr++] = 0x81;
        buf[ptr++] = 0x87; // SEND_IMMEDIATE;
        ftdi_write_data(&ftdi, buf, ptr);

        ftdi_read_data(&ftdi, &gpio_val, 1);
        if ((Pin::DI & gpio_val) != 0) break;

        ftdi_tcioflush(&ftdi);
        usleep(20);
    }
    if (j > 254)
    {
        std::cout << "Chip not answer\n";
        return -1;
    }


    ptr=0;
    for (int j = 0; j < 8; j++)
    {
        buf[ptr++] = 0x80;
        buf[ptr++] =  0x00; // CS High
        buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
    }

    buf[ptr++] = 0x87; // SEND_IMMEDIATE;
    if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
    {
        std::cout << "Write reading size failed\n";
        return -1;
    }
    usleep(20);

    // Set CS = LOW - STOP
    ptr = 0;
    for (int j = 0; j < 8; j++)
    {
        buf[ptr++] = 0x80;
        buf[ptr++] = 0x00; // CS low
        buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
    }

    buf[ptr++] = 0x87; // SEND_IMMEDIATE;
    if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
    {
        std::cout << "Write reading size failed\n";
        return -1;
    }

    usleep(20);
    //ftdi_tcioflush(&ftdi);
    return 0;
}

static int ft232MWSendCommandAndAddress(bool command1, bool command0, uint16_t address, uint8_t algorithm, bool stopped){
    unsigned char buf[64];
    uint32_t ptr = 0;
    uint8_t addressHi, addressLo, addrLenght, lenLo, lenHi;
    
    addrLenght = algorithm & 0x0f;  // address lenght for 8 bit mode

    if ((algorithm & 0x10) != 0) addrLenght--;  // address lenght for 16 bit mode

    addressHi = (address & 0xff00) >> 8;
    addressLo = address & 0x00ff;
printf ("addrlen=%d\n",addrLenght);
printf("addr hi= %d lo=%d\n", addressHi, addressLo);

    // 1. Set CS = HIGH (0x08), CLK = LOW, DO = LOW
    buf[ptr++] = 0x80; // Set Data Bits Low Byte
    buf[ptr++] = Pin::CS; // CS high
    buf[ptr++] = Pin::SK | Pin::DO | Pin::CS; // Directions

    // Send start
    buf[ptr++] = 0x1B; 
    buf[ptr++] = 0x00; // 0x00 = 1 bit (0x00 + 1)
    buf[ptr++] = 1;

    // Send command - two bits
    buf[ptr++] = 0x1B; 
    buf[ptr++] = 0x00; // 0x00 = 1 bit (0x00 + 1)
    buf[ptr++] = command1;    
    
    buf[ptr++] = 0x1B; 
    buf[ptr++] = 0x00; // 0x00 = 1 bit (0x00 + 1)
    buf[ptr++] = command0;
    
    lenLo = addrLenght - 1; //Address lenght without command lenght

    if (lenLo > 7)
    {
        lenLo = 7;
        lenHi = addrLenght - 9;
printf("addr lenHi= %x lenLo=%x\n", lenHi, lenLo);
printf("addr addressHi=%02X addressLo=%02X", addressHi, addressLo);
        buf[ptr++] = 0x1B;
        buf[ptr++] = lenHi; //Address lenght without command lenght
        addressHi = swap_byte(addressHi) >> (7 - lenHi);
        buf[ptr++] = addressHi;
    }

    //Send address addrLenght bits (addrLenght <= 8)
    buf[ptr++] = 0x1B; 
    buf[ptr++] = lenLo; //Address lenght without command lenght
    addressLo = swap_byte(addressLo) >> (7 - lenLo);
    buf[ptr++] = addressLo;
printf("rotated addr addressHi=%02X addressLo=%02X", addressHi, addressLo);
std::printf(" %08b ", addressHi); // Выведет: 00000101
std::printf(" %08b ", addressLo); // Выведет: 00000101
std::printf("\n"); // Выведет: 00000101

    if (stopped)
    {
        // Set SK = LOW
        for (int i = 0; i < 8; i++)
        {
            buf[ptr++] = 0x80;
            buf[ptr++] = Pin::CS;
            buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
        }

        // Set CS = LOW
        for (int i = 0; i < 8; i++)
        {
            buf[ptr++] = 0x80;
            buf[ptr++] = 0x00; // CS low
            buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
        }
    }
    buf[ptr++] = SEND_IMMEDIATE; // 0x87

    if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
    {
        return -1;
    }
    return 0;
}

int ft232MWWriteEnable(uint8_t algorithm)
{
    int ret = 0;
    uint16_t enAddress;

    enAddress = 0x03 << ((algorithm & 0x0f) - 3);
    if ((algorithm & 0xf0) == 0) enAddress = enAddress << 1;
    ret = ft232MWSendCommandAndAddress(0, 0, enAddress, algorithm, 1);//Write enable
    return ret;
}

int ft232MWWriteDisable(uint8_t algorithm)
{
    int ret = 0;

    ret = ft232MWSendCommandAndAddress(0, 0, 0x0000, 0x17, 1);//Write disable
    return ret;
}

int ft232MWReadBlock(uint8_t *buffer, uint16_t startAddr, uint16_t sizeToRead, uint8_t algorithm)
{
    unsigned char buf[16384] = {0};
    uint32_t ptr = 0;
    uint8_t nextBytes, i, j, mask;
    if ((algorithm & 0x10) == 0) nextBytes = algorithm & 0x0f; // 8 bit mode
    else // 16 bit mode
    {
        nextBytes = algorithm & 0x0f - 1;
        startAddr = startAddr >> 1;
    }

    // calculating high mask address
    for (i = 0; i < nextBytes - 1; i++)
    {
        mask = mask << 1;
    }

    ft232MWSendCommandAndAddress(1, 0, startAddr, algorithm, 0);
    // Reading 16 / 8 bits
    buf[ptr++] = 0x24; // or 0x20 MPSSE_DO_READ
    sizeToRead--;
    buf[ptr++] = sizeToRead & 0xFF;
    buf[ptr++] = sizeToRead >> 8;
    // SET SK = LOW
    for (int i = 0; i < 8; i++)
    {
        buf[ptr++] = 0x80;
        buf[ptr++] = Pin::CS;
        buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
    }

    // Set CS = LOW
    for (int i = 0; i < 8; i++)
    {
        buf[ptr++] = 0x80;
        buf[ptr++] = 0x00; // CS low
        buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
    }
    
    buf[ptr++] = 0x87; // SEND_IMMEDIATE;
    
   if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
   {
      std::cout << "Write reading size failed\n";
      return -1;
   }
   
   // Reading FTDI buffer
   int bytes_read = 0;
   int total_read = 0;
   while (total_read < sizeToRead) {
       bytes_read = ftdi_read_data(&ftdi, buffer + total_read, sizeToRead - total_read + 1);
        if (bytes_read < 0) break;
        total_read += bytes_read;
        usleep(5000);
    }

    // SWAP bytes
    //if ((algorithm & 0x10) != 0)
    //{
        for (int i = 0; i < sizeToRead; i = i + 2)
        {
            j = buffer[i + 1];
            buffer[i + 1]  = buffer[i];
            buffer[i] = j;
        }
    //}

   ftdi_tcioflush(&ftdi);
   usleep(5000);
   return 0;
}

int ft232hMWEraseAll(uint8_t algorithm)
{
    uint16_t eraseAddress;

    ft232MWWriteEnable(algorithm);

    eraseAddress = 0x02 << ((algorithm & 0x0f) - 3);
    if ((algorithm & 0xf0) == 0) eraseAddress = eraseAddress << 1;

    ft232MWSendCommandAndAddress(0, 0, eraseAddress, algorithm, 1); //Erase all
    if (ft232hMWWaitForReady() != 0) return -1;
    return 0;
}

int ft232MWWriteBlock(uint8_t *buffer, uint16_t startAddr, uint16_t sizeToWrite, uint8_t algorithm)
{
    unsigned char buf[2048] = {0};
    uint32_t ptr = 0;
    uint8_t nextBytes, i, j, mask, busyStatus;
    if ((algorithm & 0x10) == 0) nextBytes = algorithm & 0x0f;
    else
    {
        nextBytes = algorithm & 0x0f - 1;
        startAddr = startAddr >> 1;
    }

    // calculating high mask address
    for (i = 0; i < nextBytes - 1; i++)
    {
        mask = mask << 1;
    }
    //ft232MWSendCommandAndAddress(0, 0, 0x0030, algorithm, 1); //Write enable


    for (i = 0; i < sizeToWrite; i = i + 2)
    {
        //ft232MWSendCommandAndAddress(0, 0, 0x0030, algorithm, 1); //Write enable
        usleep(100);
        ptr = 0;
        ft232MWSendCommandAndAddress(0, 1, startAddr + (i >> 1), algorithm, 0);
        buf[ptr++] = MPSSE_DO_WRITE | MPSSE_WRITE_NEG; //0x11

        buf[ptr++] = 0x01;
        buf[ptr++] = 0x00;

        buf[ptr++] = buffer[i + 1];
        buf[ptr++] = buffer[i];

        // SET SK = LOW
        for (int i = 0; i < 8; i++)
        {
            buf[ptr++] = 0x80;
            buf[ptr++] = Pin::CS;
            buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
        }

        // sET CS = LOW
        for (int i = 0; i < 8; i++)
        {
            buf[ptr++] = 0x80;
            buf[ptr++] = 0x00; // CS low
            buf[ptr++] = Pin::SK | Pin::DO | Pin::CS;
        }

        buf[ptr++] = 0x87; // SEND_IMMEDIATE;

        if (ftdi_write_data(&ftdi, buf, ptr) != (int)ptr)
        {
            std::cout << "Write reading size failed\n";
            return -1;
        }

        if (ft232hMWWaitForReady() != 0) return -1;
    }

    ftdi_tcioflush(&ftdi);
    return 0;
}

// -----------------------------
// REMOVE FOR USING AS LIBRARY
// -----------------------------
/*int main(void)
{
    int x;
    std::cout << "Please select the protocol:\n0 - I2C\n1 - SPI\n2 - Microwire\n";
    std::cin >> x;
    
    if(x==0)
    {
        std::cout << "I2C protocol selected.\n\n";
        std::cout << "Starting FT232H Fast Buffered I2C test with LED control...\n";

        if (initFt232h() != 0)
        {
            return -1;
        }
    
        if (ft232hSetSpeedI2C(20) != 0) 
        { //20,100,400,750
           ftdi_usb_close(&ftdi);
           return -1;
        }

        std::cout << "I2C Initialized. Press Enter to send write transaction...\n";
        std::cin.get();

        uint8_t dev_addr = 0x50; 
        uint8_t tx_buf[512];
        for (int i = 0; i< 256; i++)
        {
		    tx_buf[i] = i;
        }
        for (int i = 256; i< 512; i++)
        {
		    tx_buf[i] = 255 - i;
        }

        if (ft232I2cBlockWrite(tx_buf, 0x00, 512, 16, 0x71) == 0) 
        {
            std::cout << "Success! Transaction completed.\n";
        } 
        else 
        {
           std::cout << "Transaction failed!\n";
        }
        
        ftdi_usb_reset(&ftdi);
        ftdi_usb_close(&ftdi);
    
        std::cout << "Read stored data... Press Enter to read...\n";
        std::cin.get();
   
        if (initFt232h() != 0)
        {
            return -1;
        }
   
        if (ft232hSetSpeedI2C(20) != 0) 
        { // 100 kHz
            ftdi_usb_close(&ftdi);
            return -1;
        }
        
        uint8_t read_buf[512]={0xff};
        if (ft232I2cBlockRead(read_buf,0x00, 512, 0x71) == 0)  
        {
            //DUMP
            for (int j = 0; j<32; j++)
            {
                printf ("0x%02X0 | ", j);
                for (int i=0; i<16; i++)
                {
                     printf ("%02X ", read_buf[j*16+i]);
                }
            printf("\n");
            }

       }
       // Cleanup and close
       closeFt232h();
    }
    
    if (x==1)
    {
       std::cout << "SPI protocol selected.\n\n";
       // Setup MPSSE; Operation code followed by 0 or more arguments.
       unsigned int icmd = 0;
       uint8_t buf[256] = {0};
       if (initFt232h() != 0) return -1;
       ft232hSetSpeedSPI(1000);
   
       ft232h_CS_LO();
   
       uint8_t command[8];
       command[0] = 0x9f;
       command[1] = 0x00;
       command[2] = 0x00;
       command[3] = 0x00;
       ft232WriteNbytes(&command[0], 1);
       //ft232hSetSpeed(0x00ff);
       ft232ReadNbytes(&buf[0],3);
       ft232h_CS_HI();
    
       std::cout << "Reading DEVICE ID\n";
       std::cout << "Answer: " << std::hex << (unsigned int)buf[0] << " "<< std::hex << (unsigned int)buf[1]<<" " << std::hex << (unsigned int)buf[2]<<" " << std::hex << (unsigned int)buf[3]<<'\n'; 
       std::cin.get();
       std::cout << "Reading SECTOR 0\n";
       uint8_t read_buf[65536]={0};
       ft232hSetSpeedSPI(1000);
       ft232h_CS_LO();
       command[0] = 0x03;
       command[1] = 0x00;
       command[2] = 0x00;
       command[3] = 0x00;
       
       ft232WriteNbytes(&command[0], 4);
       ft232ReadNbytes(&read_buf[0],65536);
       ft232h_CS_HI();
    
       //DUMP
       for (int j = 0; j<4096; j++)
       {
           printf ("0x%02X0 | ", j);
           for (int i=0; i<16; i++)
           {
                printf ("%02X ", read_buf[j*16+i]);
           }
           printf (" | ");
           for (int i=0; i<16; i++)
           {
                printf ("%C", read_buf[j*16+i]);
           }
       printf("\n");
       }
       // close ftdi
       closeFt232h();
       return 0;
    }
    
    if (x==2)
    {
        uint8_t read_buf[65536]={0};
        uint8_t read_buf1[65536]={0};
        std::cout << "MicroWire protocol selected.\n\n";
        if (initFt232hMW() != 0) return -1;
        ft232hSetSpeedSPI(20);

        ft232MWWriteEnable(0x17); //0x17 -> 93c46

        ft232hMWEraseAll(0x17);

        uint8_t buf_wr[256] = {0};
        for (int i = 0; i< 256; i++)
        {
            buf_wr[i] = i;
        }
        ft232MWWriteBlock(&buf_wr[0x10],0x10,32,0x17);
        ft232MWWriteBlock(&buf_wr[0x40],0x40,32,0x17);
        ft232MWWriteDisable(0x17);

        int j = 1;
        for (j = 0; j < 8; j++)
        {
            ft232MWReadBlock(&read_buf[j*16], j*16, 16, 0x17);
        }
        
       //DUMP
       for (int j = 0; j<10; j++)
       {
           printf ("0x%02X0 | ", j);
           for (int i=0; i<16; i++)
           {
                printf ("%02X ", read_buf[j*16+i]);
           }
           printf (" | ");
           for (int i=0; i<16; i++)
           {
               if ((read_buf[j*16+i] > 32) & (read_buf[j*16+i] < 128)) printf ("%C", read_buf[j*16+i]);
                else printf(" ");
           }
       printf("\n");
       }

        
            
      // close ftdi
      closeFt232h();
        return 0;
    }
    
    return 0;
}
*/