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

extern struct ch347_priv *priv;

int ProgDeviceInit( u8 deviceType, u8 chipType, u8 i2cSpeed )
{
    int ret;
    if (deviceType < 2)  ret = ch341a_init(chipType, i2cSpeed);
    if (deviceType == 2) ret = ch347_spi_init(chipType, i2cSpeed);
    return ret;
}

int ProgDeviceClose( u8 deviceType )
{
    if (deviceType < 2)  ch341a_spi_shutdown();
    if (deviceType == 2) ch347_spi_shutdown();
    return 0;
}


SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Manual_Mode( void )
{
	return 0;
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_One_Byte( u8  data, u8 deviceType )
{
    if (deviceType < 2) return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(1, 0, &data, NULL);
    else return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, &data, 1);
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_High( u8 deviceType )
{
    if (deviceType < 2) return (SPI_CONTROLLER_RTN_T)enable_pins(false);
    else return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 1);
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_Low( u8 deviceType )
{
    if (deviceType < 2) return (SPI_CONTROLLER_RTN_T)enable_pins(true);
    else return (SPI_CONTROLLER_RTN_T)ch347_set_cs(priv, 0, 0);
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Read_NByte( u8 *ptr_rtn_data, u32 len, SPI_CONTROLLER_SPEED_T speed, u8 deviceType )
{
    if (deviceType < 2) return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(0, len, NULL, ptr_rtn_data);
    else return (SPI_CONTROLLER_RTN_T)ch347_spi_rx(priv, ptr_rtn_data, len);
}

SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_NByte( u8 *ptr_data, u32 len, SPI_CONTROLLER_SPEED_T speed, u8 deviceType )
{
    if (deviceType < 2) return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(len, 0, ptr_data, NULL);
    else return (SPI_CONTROLLER_RTN_T)ch347_spi_tx(priv, ptr_data, len);
}

#if 0
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Xfer_NByte( u8 *ptr_data_in, u32 len_in, u8 *ptr_data_out, u32 len_out, SPI_CONTROLLER_SPEED_T speed )
{
	return (SPI_CONTROLLER_RTN_T)ch341a_spi_send_command(len_out, len_in, ptr_data_out, ptr_data_in);
}
#endif
/* End of [spi_controller.c] package */
