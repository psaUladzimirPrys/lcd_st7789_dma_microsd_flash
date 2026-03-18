/*
 * ui_display.h
 *
 *  Created on: 30 янв. 2026 г.
 *      Author: priss
 */

#ifndef UI_DISP_H_
#define UI_DISP_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "glib.h"
#include "adafruit_st7789_spi_config.h"
#include "adafruit_st7789.h"
#include "img_storage.h"

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

void disp_Init(void);
void disp_Update(void);
void disp_TurnOn(void);
void disp_TurnOff(void);

void disp_EraseImage(int16_t x,
                     int16_t y,
                     int16_t width,
                     int16_t height,
                     uint16_t bg_color);

void disp_DrawImage(int16_t x,
                    int16_t y,
                    img_storage_id_t img_id);





#ifdef __cplusplus
}
#endif

#endif /* UI_DISP_H_ */
