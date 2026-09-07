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

#ifndef FT232ALL_H
#define FT232ALL_H
#include <ftdi.h>

#ifdef __cplusplus
extern "C" {
#endif
//ALL PROTOCOLS
int initFt232h(void);
void closeFt232h(void);
int ft232hGetDescriptor(uint8_t *buf);

//SPI
int ft232hSetSpeedSPI(uint16_t speed_khz);
int ft232h_CS_LO(void);
int ft232h_CS_HI(void);
int ft232WriteNbytes(uint8_t *buffer, uint32_t sizeToWrite);
int ft232ReadNbytes(uint8_t *buffer, uint32_t sizeToRead);

//I2C
int ft232hSetSpeedI2C(uint16_t speed_khz);
int ft232I2cBlockWrite(uint8_t *data, uint32_t address, uint32_t blockSize, uint32_t sectorSize, uint8_t algorithm);
int ft232I2cBlockRead(uint8_t *data, uint32_t address, uint32_t blockSize, uint8_t algorithm);

//MicroWire
int initFt232hMW(void);
int ft232MWWriteEnable(uint8_t algorithm);
int ft232MWWriteDisable(uint8_t algorithm);
int ft232hMWEraseAll(uint8_t algorithm);
int ft232MWReadBlock(uint8_t *buffer, uint16_t startAddr, uint16_t sizeToRead, uint8_t algorithm);
int ft232MWWriteBlock(uint8_t *buffer, uint16_t startAddr, uint16_t sizeToWrite, uint8_t algorithm);

#ifdef __cplusplus
}
#endif
#endif // FT232ALL_H

