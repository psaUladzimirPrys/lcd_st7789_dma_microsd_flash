/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "hglobal.h"
#include "fuim.h"
#include "fmnu.h"
#include "fmnu_str.h"

Byte fmnu_str_Texno_Menu_STRING[] = "1.2";
Byte * fmnu_str_GetVerTexnoMenu(void)
{  
  return  (Byte * )(&fmnu_str_Texno_Menu_STRING[0]);
}




Byte const fmnu_str_List[FMNU_LIST_ITEMS_LAST_STRING] =
{
    IMG_ID_PROPERTY_1_VARIANT9_4
   ,IMG_ID_PROPERTY_1_VARIANT8_6

   ,IMG_ID_A
};
