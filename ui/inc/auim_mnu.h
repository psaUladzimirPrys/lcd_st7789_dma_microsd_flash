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


/*==========================================================================*/
/* G L O B A L   R E F E R E N C E S                                        */
/*==========================================================================*/

struct fmnu_MenuStruct;

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/

/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

typedef enum {

  AUIM_MNU_INDEX_CONFIG_MENU     = 0
 ,AUIM_MNU_INDEX_IDLE_MENU
 ,AUIM_MNU_INDEX_PAIRING_MENU

 ,AUIM_MNU_MAX_MENUS

} menu_index_enum;



enum config_menu_fields_index_enum  {
  AUIM_MNU_CONFIG_FIELD_SERIAL_NUMBER = 0
 ,AUIM_MNU_CONFIG_FIELD_FIRMWARE_VERSION
 ,AUIM_MNU_CONFIG_FIELD_REFERENCE_NUMBER
 ,AUIM_MNU_CONFIG_FIELD_STRAIN_GAUGE_STATUS
 ,AUIM_MNU_CONFIG_FIELD_CALIBRATION_CONSTANT
 ,AUIM_MNU_CONFIG_MAX_FIELD
};

enum idle_menu_fields_index_enum  {
   AUIM_MNU_IDLE_FIELD_STATUS = 0
  ,AUIM_MNU_IDLE_MAX_FIELD
};


enum pairing_menu_fields_index_enum  {
   AUIM_MNU_PAIRING_FIELD_CODE = 0
  ,AUIM_MNU_PAIRING_MAX_FIELD
};

typedef enum {

   AUIM_MNU_TOP_FIXED_FIELDS
  ,AUIM_MNU_BOTTON_FIXED_FIELDS

  ,AUIM_MNU_MAX_FIXED_FIELDS
  
}menu_fixed_fields_index_enum;


typedef enum {

   AUIM_IDLE_MNU_LEFT_BUTTON_FIELD = 0
  ,AUIM_IDLE_MNU_RIGHT_BUTTON_FIELD
  ,AUIM_PAIRING_MNU_RIGHT_BUTTON_FIELD

  ,AUIM_CONFIG_MNU_LEFT_BUTTON_FIELD
  ,AUIM_CONFIG_MNU_RIGHT_BUTTON_FIELD

  ,AUIM_MNU_MAX_BUTTON_FIELDS

}menu_buttons_index_enum;



#endif /* UI_INC_AUIM_MNU_H_ */
