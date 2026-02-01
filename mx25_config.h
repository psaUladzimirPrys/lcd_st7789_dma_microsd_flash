/*
 * mx25_config.h
 *
 * Hardware-level driver for Macronix MX25xx SPI NOR Flash
 *
 * Supported devices:
 *   MX25Lxxxx / MX25Rxxxx family
 *
 * Transport:
 *   SPI (via Silicon Labs SPIDRV or USART SPI)
 */

#ifndef MX25_CONFIG_H_
#define MX25_CONFIG_H_

/*
 * mx25_config.h
 *
 * Board / platform specific configuration for MX25 SPI NOR Flash
 *
 * This file must be adapted to your hardware.
 */

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "spidrv.h"
#include "em_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SPI driver handle
 * ========================================================================== */

/*
 * This handle must be defined and initialized in application code
 * (usually in app.c or board init file).
 *
 * Example:
 *
 *   SPIDRV_HandleData_t mx25_spi_handle;
 *
 *   SPIDRV_Init_t initData = {
 *     .port            = USART1,
 *     .bitRate         = 8000000,
 *     .frameLength     = 8,
 *     .dummyTxValue    = 0xFF,
 *     .type            = spidrvMaster,
 *     .bitOrder        = spidrvBitOrderMsbFirst,
 *     .clockMode       = spidrvClockMode0,
 *     .csControl       = spidrvCsControlApplication,
 *     .slaveStartMode  = spidrvSlaveStartImmediate
 *   };
 *
 *   SPIDRV_Init(&mx25_spi_handle, &initData);
 */
extern SPIDRV_HandleData_t mx25_spi_handle_data;

/* ============================================================================
 * Chip Select (CS) pin configuration
 * ========================================================================== */
#ifndef MX25_CS_PORT 
#define MX25_CS_PORT      gpioPortC
#endif
#ifndef MX25_CS_PIN  
#define MX25_CS_PIN       3
#endif

/* ============================================================================
 * Optional pins (not used by default)
 * ========================================================================== */
/*
 * Define these only if your board connects them.
 * Otherwise they can be ignored.
 */
//#define MX25_WRITE_PROTECT_ENABLE

#ifdef  MX25_WRITE_PROTECT_ENABLE

/* ============================================================================
 * Write Protection (WP) pin configuration
 * ========================================================================== */
/* Write Protect (active low) */
#ifndef MX25_WP_PORT
#define MX25_WP_PORT        gpioPortC
#endif
#ifndef MX25_WP_PIN
#define MX25_WP_PIN         4
#endif

/* ============================================================================
 * Hardware Reset (RST) pin configuration
 * ========================================================================== */
/* Hold / Reset (active low) */
#ifndef MX25_RST_PORT 
#define MX25_RST_PORT       gpioPortC
#endif
#ifndef MX25_RST_PIN 
#define MX25_RST_PIN        5
#endif

#endif

/* ============================================================================
 * GPIO helpers
 * ========================================================================== */
/* CS control macros */
#define MX25_CS_LOW()       GPIO_PinOutClear(MX25_CS_PORT, MX25_CS_PIN)
#define MX25_CS_HIGH()      GPIO_PinOutSet(MX25_CS_PORT, MX25_CS_PIN)
/* ============================================================================
 * SPI
 * ========================================================================== */


/* ============================================================================
 * Operation timeout
 * ========================================================================== */

/*
 * Timeout for program / erase operations.
 * Unit: number of polling iterations.
 *
 * Must be long enough for:
 *  - sector erase (typ. 150ms, max ~400ms)
 *  - page program (typ. <1ms)
 *
 * With LDMA + low-power sleep, this is safe.
 */
#define MX25_OP_TIMEOUT   (800000UL)

/* ============================================================================
 * SPI timing / limits
 * ========================================================================== */

/* Maximum supported SPI clock for MX25L/MX25R */
#define MX25_SPI_MAX_FREQ       38400000UL

/* Recommended default */
#define MX25_SPI_DEFAULT_FREQ   MX25_SPI_MAX_FREQ    //19200000UL

/* ============================================================================
 * Driver behavior configuration
 * ========================================================================== */

/*
 * Enable parameter checking (NULL pointers, zero lengths)
 * Set to 0 to remove checks for maximum performance
 */
#define MX25_CFG_PARAM_CHECK   1

/*
 * Busy-wait delay hook (optional)
 * Useful for RTOS yield or low-power wait
 *
 * Example:
 *   #define MX25_DELAY_HOOK() osDelay(1)
 */
/*
 * Optional delay hook used while polling WIP.
 * Can be replaced with RTOS delay or sleep.
 * #define MX25_DELAY_HOOK()  __NOP()
 */ 
#ifndef MX25_DELAY_HOOK 
//  #define MX25_DELAY_HOOK()  do {} while (0)
#define MX25_DELAY_HOOK()  sl_sleeptimer_delay_millisecond(10)
//#define MX25_DELAY_HOOK()
#endif

#ifdef __cplusplus
}
#endif

#endif /* MX25_CONFIG_H_ */
