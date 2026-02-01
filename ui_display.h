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



  extern const unsigned char gImage_cat_image[64808];
  extern const unsigned char gImage_bird_image[64808];
  extern const unsigned char gImage_cute_image[64808];
  extern const unsigned char gImage_cactus_plants[64808];
  extern const unsigned char gImage_nature_image[64808];

#define FLASH_ADDR_IMAGE_CAT_IMAGE 0
#define FLASH_ADDR_BIRD_IMAGE      sizeof(gImage_cat_image)
#define FLASH_ADDR_CUTE_IMAGE      64808 + FLASH_ADDR_BIRD_IMAGE
#define FLASH_ADDR_CACTUS_PLANTS   64808 + FLASH_ADDR_CUTE_IMAGE
#define FLASH_ADDR_NATURE_IMAGE    64808 + FLASH_ADDR_CACTUS_PLANTS


/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

void disp_init(void);

void disp_process_action(void);


#ifdef __cplusplus
}
#endif

#endif /* UI_DISPLAY_H_ */
