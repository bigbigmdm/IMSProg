/*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_controller.h
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

#ifndef __SPI_CONTROLLER_H__
#define __SPI_CONTROLLER_H__

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */

/* MACRO DECLARATIONS ---------------------------------------------------------------- */

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum{
	SPI_CONTROLLER_SPEED_SINGLE = 0,
	SPI_CONTROLLER_SPEED_DUAL,
	SPI_CONTROLLER_SPEED_QUAD

} SPI_CONTROLLER_SPEED_T;

typedef enum{
	SPI_CONTROLLER_RTN_NO_ERROR = 0,
	SPI_CONTROLLER_RTN_SET_OPFIFO_ERROR,
	SPI_CONTROLLER_RTN_READ_DATAPFIFO_ERROR,
	SPI_CONTROLLER_RTN_WRITE_DATAPFIFO_ERROR,
	SPI_CONTROLLER_RTN_DEF_NO
} SPI_CONTROLLER_RTN_T;


typedef enum{
	SPI_CONTROLLER_MODE_AUTO = 0,
	SPI_CONTROLLER_MODE_MANUAL,
	SPI_CONTROLLER_MODE_NO
} SPI_CONTROLLER_MODE_T;

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Manual_Mode( void )
 * PURPOSE : To provide interface for enable SPI Controller Manual Mode Enable.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Manual_Mode( void );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_One_Byte( u8  data )
 * PURPOSE : To provide interface for write one byte to SPI bus.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : data - The data variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_One_Byte( u8  data );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_WRITE_NBYTES( u8                        *ptr_data,
 *                                                             u32                       len,
 *                                                             SPI_CONTROLLER_SPEED_T    speed )
 * PURPOSE : To provide interface for write N bytes to SPI bus.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : ptr_data  - The data variable of this function.
 *           len   - The len variable of this function.
 *           speed - The speed variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_NByte( u8 *ptr_data, u32 len, SPI_CONTROLLER_SPEED_T speed );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_READ_NBYTES( u8                         *ptr_rtn_data,
 *                                                            u8                         len,
 *                                                            SPI_CONTROLLER_SPEED_T     speed     )
 * PURPOSE : To provide interface for read N bytes from SPI bus.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : len       - The len variable of this function.
 *           speed     - The speed variable of this function.
 *   OUTPUT: ptr_rtn_data  - The ptr_rtn_data variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Read_NByte( u8 *ptr_rtn_data, u32 len, SPI_CONTROLLER_SPEED_T speed );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_Low( void )
 * PURPOSE : To provide interface for set chip select low in SPI bus.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_Low( void );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_High( void )
 * PURPOSE : To provide interface for set chip select high in SPI bus.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *------------------------------------------------------------------------------------
 */
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_High( void );

#if 0
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Xfer_NByte( u8 *ptr_data_in, u32 len_in, u8 *ptr_data_out, u32 len_out, SPI_CONTROLLER_SPEED_T speed );
#endif

#endif /* ifndef __SPI_CONTROLLER_H__ */
/* End of [spi_controller.h] package */
