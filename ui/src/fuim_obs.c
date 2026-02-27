/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/


#include "fuim_obs.h"
#include "fmnu.h"
#include "aukh.h"
#include "auim_mnu.h"
#include "auim_api.h"




/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/


/*============================================================================*/
/*
   @func    Calls an observer function of a menu or an indicator (See
            GetFunction fields)

   @rdesc   The result of the observer function with ID index.

   @comm    An observer may also return a string pointer, in which
            case the result needs to te be re-cast to '(Byte *)'.

   @design  The return values of observer functions that return a Byte are
            casted at the end of this function, before returning. When an
            observer function does return another type the return value is
            immediately casted, after which the functio is left.

*/
osdFieldValue fuim_Observer (Byte index)
/*=======================================================================*/
{
   Byte result;

   switch (index)

   {
      case AUIM_GET_MAIN_VOLUME:
         result = 100;//psnd_GetVolume_control();
         break;

      case AUIM_NO_GET_FUNCTION:
      default:
         result = 0;
         break;
   }

   return ((osdFieldValue) result);
}
/*============================================================================*/
/*
   @rdesc   The result of the observer function with ID index.

   @comm    An observer may also return a string pointer, in which
            case the result needs to be re-cast to '(Byte *)'.

   @design  The return values of observer functions that return a Byte are
            casted at the end of this function, before returning. When an
            observer function does return another type the return value is
            immediately casted, after which the function is left.

*/
fuim_Validity fuim_ValidityFunction (Byte index)
{
  fuim_Validity result;
  switch (index)
  {
    case AUIM_FIELD_SPACER_VALIDITY_FUNCTION:
    case AUIM_FIELD_SEPARATOR_VALIDITY_FUNCTION :
    {
      result = FUIM_VALIDITY_VISIBLE;
    } break;

    case  AUIM_MENU_VALIDITY_FUNCTION :
    case  AUIM_FIELD_VALIDITY_FUNCTION :
    {
      result = FUIM_VALIDITY_SELECTABLE;
    }break;

    case AUIM_FIELD_PICTURE_STORE_VALIDITY_FUNCTION:
    {
          result = FUIM_VALIDITY_VISIBLE;
    }break;
   case AUIM_FIELD_MANUAL_TUNE_WSB_VALIDITY_FUNCTION:
   {
       result = FUIM_VALIDITY_SELECTABLE;
   }break;
    case AUIM_FIELD_EDIT_VALIDITY_FUNCTION :
    case AUIM_MENU_TIMER_PROG_NUMBER_VALIDITY_FUNCTION :
    {
      result = FUIM_VALIDITY_GRAYEDOUT;
    }break;

     case AUIM_INDICATOR_NUMBER_PROGRAMM_VALIDITY_FUNCTION:
    {
      result = FUIM_VALIDITY_SELECTABLE;

    }break;

    default:
         result = FUIM_VALIDITY_NOTPRESENT;
         break;
  }
 return ((fuim_Validity) result);

}

/*============================================================================*/
/*
   @func    Calls an action handler (See ...DialogKeys arrays)

   @rdesc   The result of the action handler

   @comm    The value given to the action handler function may also be a
            string pointer which has been cast into an osdFieldValue.

*/
/* @parm Function ID */  /* @parm given to the action handler function */
/*============================================================================*/
osdFieldValue fuim_ActionHandler(Byte index,  osdFieldValue value)
{
   Byte result = AU_KEY_PROCESSED;
   Byte byte_value = 0;

  switch (index)
  {
    case AUIM_DISPLAY_CONFIG_MENU             :
      fmnu_Activate(AUIM_MNU_INDEX_CONFIG_MENU);
      break;

    case AUIM_NO_ACTION_FUNCTION:                 // Fall through
    default:
      byte_value = (Byte) value;
      if (byte_value == 0)  result = AU_KEY_PROCESSED;
        break;
  }

    return ((osdFieldValue) result);
}


