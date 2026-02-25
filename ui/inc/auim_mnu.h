/*
 * auim_mnu.h
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: priss
 */

#ifndef UI_INC_AUIM_MNU_H_
#define UI_INC_AUIM_MNU_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "img_storage.h"
#include "fmnu.h"
#include "fmnu_str.h"

/*==========================================================================*/
/* G L O B A L   R E F E R E N C E S                                        */
/*==========================================================================*/


/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/

/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

typedef enum {

  AUIM_MNU_INDEX_CONFIG_MENU     = 0
  //,AUIM_MNU_INDEX_SOUND_MENU
  //,AUIM_MNU_INDEX_PICTURE_MENU
  //,AUIM_MNU_INDEX_PROGRAMME_TUNING_MENU
  //,AUIM_MNU_INDEX_TV_SETUP_MENU
  //,AUIM_MNU_INDEX_AUTO_SEARCH_MENU
  //,AUIM_MNU_INDEX_MANUAL_SEARCH_MENU
  //,AUIM_MNU_INDEX_TIMERS_MENU
  //,AUIM_MNU_INDEX_EDIT_MENU
  ,AUIM_MNU_MAX_MENUS

} menu_index_enum;



enum config_menu_field_index_enum  {
  AUIM_MNU_CONFIG_FIELD_SERIAL_NUMBER = 0
 ,AUIM_MNU_CONFIG_FIELD_FIRMWARE_VERSION
 ,AUIM_MNU_CONFIG_FIELD_REFERENCE_NUMBER
 ,AUIM_MNU_CONFIG_FIELD_STRAIN_GAUGE_STATUS
 ,AUIM_MNU_CONFIG_FIELD_CALIBRATION_CONSTANT
 ,AUIM_MNU_CONFIG_MAX_FIELD
};

extern const fmnu_MenuStruct auim_Menu[AUIM_MNU_MAX_MENUS];

#endif /* UI_INC_AUIM_MNU_H_ */
