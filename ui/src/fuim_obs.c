/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/


#include "fuim_obs.h"
#include "fmnu.h"
#include "aukh.h"
#include "auim_mnu.h"
#include "auim_api.h"
#include "fsrv.h"



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
  osdFieldValue result;

   switch (index)
   {
      case AUIM_GET_BATTERY_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetBatStatus();
      break;

      case AUIM_GET_BLE_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetBleStatus();
      break;

      case AUIM_GET_SYNC_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetSyncStatus();
      break;

      case AUIM_GET_CHARGE_BAT_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetChargeBatStatus();
      break;

      case AUIM_GET_IDLE_WAITINGS_STATUS:
         result = (osdFieldValue)fsrv_DS_GetWaitingStat();
      break;     

      case AUIM_GET_PAIRING_CODE:
         result = (osdFieldValue)fsrv_GetPairingCode();
      break; 

      case AUIM_GET_STRAIN_GAUSE_STATUS:
         result = (osdFieldValue)fsrv_DS_GetStrainGauseStat();
      break;

      case AUIM_GET_FW_VERSION:
         result = (osdFieldValue)fsrv_DS_GetFwVersion();
      break;

      case AUIM_GET_CALIBRATION_CONST:
         result = (osdFieldValue)fsrv_DS_GetCalibrationConst();
      break;

      case AUIM_GET_REFERENCE_NUMBER:
         result = (osdFieldValue)fsrv_DS_GetRefNumber();
      break;

      case AUIM_GET_SERIAL_NUMBER:
         result = (osdFieldValue)fsrv_DS_GetSerialNum();
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

    case AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetBatStatus() != IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetBleStatus() != IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetSyncStatus() != IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION:{
      if (fsrv_DS_GetChargeBatStatus() != IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION:
    case AUIM_FIELD_BUTTON_VALIDITY_FUNCTION:
    case AUIM_FIELD_SPACER_VALIDITY_FUNCTION:
    {
      result = FUIM_VALIDITY_VISIBLE;
    } break;

    case  AUIM_MENU_VALIDITY_FUNCTION :
    case  AUIM_FIELD_VALIDITY_FUNCTION :
    {
      result = FUIM_VALIDITY_SELECTABLE;
    }break;

    case AUIM_FIELD_EDIT_VALIDITY_FUNCTION :
    {
      result = FUIM_VALIDITY_GRAYEDOUT;
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
    case AUIM_DISPLAY_CONFIG_MENU :
        fmnu_Activate(AUIM_MNU_INDEX_CONFIG_MENU);
      break;
    case AUIM_DISPLAY_IDLE_MENU :
        fmnu_Activate(AUIM_MNU_INDEX_IDLE_MENU);
      break;
    case AUIM_DISPLAY_PAIRING_MENU:
        fmnu_Activate(AUIM_MNU_INDEX_PAIRING_MENU);
      break;
    case AUIM_ACTION_NEXT_FIELD:
        //if ( byte_value == AU_VIRTUAL_KEY_1)
           result = AU_KEY_NEXT;
     break;

    case AUIM_NO_ACTION_FUNCTION :                 // Fall through
    default: {
      byte_value = (Byte) value;
      if (byte_value == 0) {
          result = AU_KEY_PROCESSED;
      }
     } break;
  }

  return ((osdFieldValue) result);
}


