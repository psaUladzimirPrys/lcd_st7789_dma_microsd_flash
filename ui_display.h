/*
 * ui_display.h
 *
 *  Created on: 30 янв. 2026 г.
 *      Author: priss
 */

#ifndef UI_DISPLAY_H_
#define UI_DISPLAY_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "glib.h"
#include "adafruit_st7789_spi_config.h"
#include "adafruit_st7789.h"


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

void disp_init(void);

void disp_process_action(void);


#ifdef __cplusplus
}
#endif

#endif /* UI_DISPLAY_H_ */
