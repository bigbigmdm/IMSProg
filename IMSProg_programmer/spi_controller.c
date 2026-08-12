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

/*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_controller.c
 *
 * FUNCTIONS
 *
 *      SPI_CONTROLLER_Enable_Manual_Mode To provide interface for Enable SPI Controller Manual Mode.
 *      SPI_CONTROLLER_Write_One_Byte     To provide interface for write one byte to SPI bus.
 *      SPI_CONTROLLER_Write_NByte        To provide interface for write N bytes to SPI bus.
 *      SPI_CONTROLLER_Read_NByte         To provide interface for read N bytes from SPI bus.
 *      SPI_CONTROLLER_Chip_Select_Low    To provide interface for set chip select low in SPI bus.
 *      SPI_CONTROLLER_Chip_Select_High   To provide interface for set chip select high in SPI bus.
 *
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 *
 *======================================================================================
 */
#include "ch341a_spi.h"
#include "spi_controller.h"
#include "ch347.h"
#include "ft232all.h"

extern struct ch347_priv *priv;

int ProgDeviceInit( u8 deviceType, u8 chipType, u8 i2cSpeed )
{
    int ret = 0;
    uint speed = 20;
    switch (deviceType)
        {
        case 0: // CH341A v1.2
            ret = ch341a_init(chipType, i2cSpeed);
            break;
        case 1: // CH341A v1.7
            ret = ch341a_init(chipType, i2cSpeed);
            break;
        case 2: // CH347T v1.0
            ret = ch347_spi_init(chipType, i2cSpeed, false);
            break;
        case 3: // CH347T v1.1
            ret = ch347_spi_init(chipType, i2cSpeed, true);
            break;
        case 4: // FT232H v1.2
            ret = initFt232h();
            if (ret != 0) return ret;
            if  (chipType == 2) ft232hSetSpeedSPI(20);
            if ((chipType == 3) || (chipType == 4) || (chipType == 5)) ft232hSetSpeedSPI(5000);
            if ((chipType == 0) || (chipType == 6)) ft232hSetSpeedSPI(30000);
            break;
        default: //Unsupported types
            ret = -1;
            break;
        }
        if ((chipType == 1) && (deviceType == 4))
        {
            switch (i2cSpeed)
            {
            case 0:
                speed = 20;
                break;
            case 1:
                speed = 100;
                break;
            case 2:
                speed = 400;
                break;
            case 3:
                speed = 750;
                break;
            default:
                speed = 20;
                break;
            }
            ft232hSetSpeedI2C(speed);
        }
        if ((chipType == 2) && (deviceType == 4)) ft232hSetSpeedI2C(20);
    return ret;
}

int ProgDeviceClose( u8 deviceType )
{
    int ret = 0;
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        ret = ch341a_spi_shutdown();
        break;
    case 1: // CH341A v1.7
        ret = ch341a_spi_shutdown();
        break;
    case 2: // CH347T v1.0
        ch347_spi_shutdown();
        ret = 0;
        break;
    case 3: // CH347T v1.1
        ch347_spi_shutdown();
        ret = 0;
        break;
    case 4: // FT232H v1.2
        closeFt232h();
        ret = 0;
        break;
    default: //Unsupported types
        ret = -1;
        break;
    }
    return ret;
}

int getDeviceDescriptor(u8 *data, u8 deviceType)
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        ch341GetDescriptor(data);
        break;
    case 1: // CH341A v1.7
        ch341GetDescriptor(data);
        break;
    case 2: // CH347T v1.0
        ch347GetDescriptor(data);
        break;
    case 3: // CH347T v1.1
        ch347GetDescriptor(data);
        break;
    case 4: // FT232H v1.2
        ft232hGetDescriptor(data);
        break;
    default: //Unsupported types
        return -1;
        break;
    }
    return 0;
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Manual_Mode( void )
{
	return 0;
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_One_Byte( u8  data, u8 deviceType )
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(1, 0, &data, NULL);
        break;
    case 1: // CH341A v1.7
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(1, 0, &data, NULL);
        break;
    case 2: // CH347T v1.0
        return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, &data, 1);
        break;
    case 3: // CH347T v1.1
        return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, &data, 1);
        break;
    case 4: // FT232H v1.2
        return (SPI_CONTROLLER_RTN_T)ft232WriteNbytes(&data, 1);
        break;
    }
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_High( u8 deviceType )
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        return (SPI_CONTROLLER_RTN_T)enable_pins(false);
        break;
    case 1: // CH341A v1.7
        return (SPI_CONTROLLER_RTN_T)enable_pins(false);
        break;
    case 2: // CH347T v1.0
        return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 1);
        break;
    case 3: // CH347T v1.1
        return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 1);
        break;
    case 4: // FT232H v1.2
        return (SPI_CONTROLLER_RTN_T)ft232h_CS_HI();
        break;
    }
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_Low( u8 deviceType )
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        return (SPI_CONTROLLER_RTN_T)enable_pins(true);
        break;
    case 1: // CH341A v1.7
        return (SPI_CONTROLLER_RTN_T)enable_pins(true);
        break;
    case 2: // CH347T v1.0
        return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 0);
        break;
    case 3: // CH347T v1.1
        return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 0);
        break;
    case 4: // FT232H v1.2
        return (SPI_CONTROLLER_RTN_T)ft232h_CS_LO();
        break;
    }
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Read_NByte( u8 *ptr_rtn_data, u32 len, SPI_CONTROLLER_SPEED_T speed, u8 deviceType )
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(0, len, NULL, ptr_rtn_data);
        break;
    case 1: // CH341A v1.7
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(0, len, NULL, ptr_rtn_data);
        break;
    case 2: // CH347T v1.0
        return (SPI_CONTROLLER_RTN_T)ch347_spi_rx(priv, ptr_rtn_data, len);
        break;
    case 3: // CH347T v1.1
        return (SPI_CONTROLLER_RTN_T)ch347_spi_rx(priv, ptr_rtn_data, len);
        break;
    case 4: // FT232H v1.2
        return (SPI_CONTROLLER_RTN_T)ft232ReadNbytes(ptr_rtn_data, len);
        break;
    }
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_NByte( u8 *ptr_data, u32 len, SPI_CONTROLLER_SPEED_T speed, u8 deviceType )
{
    switch (deviceType)
    {
    case 0: // CH341A v1.2
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(len, 0, ptr_data, NULL);
        break;
    case 1: // CH341A v1.7
        return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(len, 0, ptr_data, NULL);
        break;
    case 2: // CH347T v1.0
        return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, ptr_data, len);
        break;
    case 3: // CH347T v1.1
        return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, ptr_data, len);
        break;
    case 4: // FT232H v1.2
        return (SPI_CONTROLLER_RTN_T)ft232WriteNbytes(ptr_data, len);
        break;
    }
}

#if 0
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Xfer_NByte( u8 *ptr_data_in, u32 len_in, u8 *ptr_data_out, u32 len_out, SPI_CONTROLLER_SPEED_T speed )
{
	return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(len_out, len_in, ptr_data_out, ptr_data_in);
}
#endif
/* End of [spi_controller.c] package */
