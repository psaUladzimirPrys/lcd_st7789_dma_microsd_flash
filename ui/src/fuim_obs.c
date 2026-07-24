/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/


#include "fuim_obs.h"
#include "fmnu.h"
#include "aukh.h"
#include "auph.h"
#include "auim_mnu.h"
#include "auim_api.h"
#include "fsrv.h"
#include "find_api.h"

#include "adc.h"
#include "params.h"
#include "aevs.h"

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
  osdFieldValue result = 0;

  if (index >= AUIM_OBSERVER_INDEX_RESERVED) {
    return (osdFieldValue)IMG_INVALID_ID;
  }

   switch (index)
   {
      case AUIM_GET_BATTERY_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetBatStatusImageId();
      break;

      case AUIM_GET_BLE_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetBleStatusImageId();
      break;

      case AUIM_GET_SYNC_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetSyncStatusImageId();
      break;

      case AUIM_GET_CHARGE_BAT_INDICATOR:
         result = (osdFieldValue)fsrv_DS_GetChargeBatLow10StatusImageId();
      break;

      case AUIM_GET_IDLE_WAITINGS_STATUS:
         result = (osdFieldValue)fsrv_DS_GetIdleWaitingStatusImageId();
      break;     

      case AUIM_GET_PAIRING_CODE:
         result = (osdFieldValue)fsrv_DS_GetBlePairingCode();
      break; 

      case AUIM_GET_PAIRING_RESULT_STATUS:
          result = (osdFieldValue)fsrv_DS_GetPairingStatusImageId();
      break;

      case AUIM_GET_PAIRING_RESULT_CANCEL:
          result = (osdFieldValue)IMG_ID_PROPERTY_1_DEFAULT_1;
      break;

      case AUIM_GET_STRAIN_GAUSE_STATUS:
         result = (osdFieldValue)fsrv_DS_GetStrainGauseStat();
      break;

      case AUIM_GET_FW_VERSION:
         result = (osdFieldValue)fsrv_DS_GetFwVersion();
      break;

      case AUIM_GET_CURRENT_DATE:
          result = (osdFieldValue)fsrv_DS_GetCurrentDate();
      break;

      case AUIM_GET_CURRENT_TIME:
          result = (osdFieldValue)fsrv_DS_GetCurrentTime();
      break;

      case AUIM_GET_AM_PM_TIME_SUFFIX_ID:
          result = (osdFieldValue)fsrv_DS_GetAmPmTimeSuffixImageId();
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

      case AUIM_GET_ERROR_CODE_NUMBER:
         result = (osdFieldValue)fsrv_DS_GetErrorCode();
      break;

      case AUIM_GET_PERF_CHECK_REQUIRED_PROMPT :
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT4_6;
      break;

      case AUIM_GET_CHARGE_BATTERY_PROMPT :
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT3_7;
      break;

      case AUIM_GET_PERFORMANCE_VALUE :
         result = (osdFieldValue)fsrv_DS_GetValidIndentations();
      break;

      case AUIM_GET_REFERENCE_VALUE:
         result = (osdFieldValue)fsrv_DS_GetValidIndentations();
      break;

      case AUIM_GET_PATIENT_VALUE:
         result = (osdFieldValue)fsrv_DS_GetValidIndentations();
      break;

      case AUIM_GET_CHARGE_BATTERY_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT3;
      break;

      case AUIM_GET_PERFORMANCE_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT9;
      break;

      case AUIM_GET_PATIENT_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT7;
      break;

      case AUIM_GET_REFERENCE_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT8;
      break;

      case AUIM_GET_ONE_INDENT_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT5;
      break;

      case AUIM_GET_TIP_ID_VALID_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT10;
      break;

      case AUIM_GET_TIP_ID_INVALID_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT2;
      break;

      case AUIM_GET_TIP_ID_USED_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_DEFAULT;
      break;

      case AUIM_GET_UNSTABLE_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT6;
      break;

      case AUIM_GET_TERMINATE_IND:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT4;
      break;

      case AUIM_GET_UNION3_OUT_IND:
         result = (osdFieldValue)IMG_ID_UNION3_OUT;
      break;

      case AUIM_GET_ERROR_IND:
         result = (osdFieldValue)IMG_ID_EC;
      break;

      case AUIM_GET_BUTTON_THREE_DOTS:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_FRAME_53;
      break;

      case AUIM_GET_BUTTON_ELLIPSE:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_RECTANGLE_201;
      break;

      case AUIM_GET_BUTTON_ONE_DOT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_ELLIPSE_15;
      break;

      case AUIM_GET_FIRMWARE_VERSION_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT12;
      break;

      case AUIM_GET_CURRENT_DATE_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT18;
      break;

      case AUIM_GET_CURRENT_TIME_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT19;
      break;

      case AUIM_GET_REFERENCE_NUMBER_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT11_4;
      break;

      case AUIM_GET_PAIRING_WAITING_PROMPT:
         result = (osdFieldValue)fsrv_DS_GetPairingWaitingStatusImageId();
      break;

      case AUIM_GET_APPROXIMATION_IND:
         result = (osdFieldValue)fsrv_DS_GetPatientApproximationImageId();
      break;

      case AUIM_GET_CANCELED_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_DEFAULT_1;
      break;
	  
      case AUIM_GET_PERFORMANCE_COMPLETE_RESULT:
         result = (osdFieldValue)fsrv_DS_GetPerformanceCompleteResultImageId();
      break;

      case AUIM_GET_REFERENCE_COMPLETE_RESULT:
         result = (osdFieldValue)fsrv_DS_GetReferenceCompleteResultImageId();
      break;

      case AUIM_GET_PATIENT_COMPLETE_RESULT:
         result = (osdFieldValue)fsrv_DS_GetPatientCompleteResultImageId();
      break;

      case AUIM_GET_PATIENT_FILED_RESULT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT3_1;
      break;

      case AUIM_GET_PERFORMANCE_RESULT:
         result = (osdFieldValue)fsrv_DS_GetPerformanceResultImageId();
      break;

      case AUIM_GET_PATIENT_RESULT:
         result = (osdFieldValue)fsrv_DS_GetPatientResult();
      break;

      case AUIM_GET_PATIENT_VALIDATING_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT5_6;
      break;

      case AUIM_GET_CALIBRATION_CONSTANT_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT10_6;
      break; 
	  
      case AUIM_GET_STRAIN_GAUGE_STATUS_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT7_6;
      break;
	  
      case AUIM_GET_SERIAL_NUMBER_PROMPT:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT6_6;
      break;

      case AUIM_GET_PERF_CHK_RECOMMENDED:
         result = (osdFieldValue)IMG_ID_PROPERTY_1_VARIANT15;
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
  fuim_Validity result = FUIM_VALIDITY_NOTPRESENT;

  switch (index)
  {

    case AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetBatStatusImageId() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetBleStatusImageId() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetSyncStatusImageId() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_PRESENT;
    }break;

    case AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetChargeBatLow10StatusImageId() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_NOTPRESENT;
    }break;

 #ifdef TEST_VALIDITY
    case AUIM_FIELD_PERFORMANCE_START_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetPerformanceStart() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_NOTPRESENT;
    }break;

    case AUIM_FIELD_PATIENT_START_VALIDITY_FUNCTION:{
      if (fsrv_DS_GetPatientStart() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_VISIBLE;
      else
        result = FUIM_VALIDITY_NOTPRESENT;
    }break;
#endif

    case AUIM_PAIRING_MENU_VALIDITY_FUNCTION: {
      if (fsrv_DS_GetPairingWaitingStatusImageId() < IMG_MAX_IDS_STORAGE_DESC_COUNT)
        result = FUIM_VALIDITY_PRESENT;
      else
        result = FUIM_VALIDITY_VISIBLE;
    }break;

    case AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION:
    case AUIM_FIELD_BUTTON_VALIDITY_FUNCTION:
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

  find_RemoveAllNotificationIndicators();

  if (index != AUIM_DISPLAY_IDLE_MENU) {
      auph_SetState(AU_MENU_STATE);
  }

  switch (index)
  {
    case AUIM_DISPLAY_CONFIGURATION_MENU:
      fmnu_Activate(AUIM_MNU_INDEX_CONFIG_MENU);
    break;

    case AUIM_DISPLAY_IDLE_MENU:
      fmnu_Activate(AUIM_MNU_INDEX_IDLE_MENU);
//      aevs_ForceChargeMenuCheck();
      auph_SetState(AU_IDLE_STATE);
    break;
    /**********************************************************/
    /*                                                        */
    /*        P A I R I N G      M E N U                      */
    /*                                                        */
    /**********************************************************/

    case AUIM_DISPLAY_PAIRING_MENU:
       fsrv_BLE_SetCloseConnection(TRUE);
       fmnu_Activate(AUIM_MNU_INDEX_PAIRING_MENU);
    break;

    case AUIM_DISPLAY_PAIRING_CANCELED_MENU:
        fsrv_BLE_SetCloseConnection(FALSE);
        fmnu_Activate(AUIM_MNU_INDEX_PAIRING_STATUS_MENU);
    break;

    case AUIM_DISPLAY_PAIRING_OK_CLOSE_MENU:
        fmnu_Activate(AUIM_MNU_INDEX_PAIRING_CLOSE_MENU);
    break;

    case AUIM_DISPLAY_PAIRING_FAILED_CLOSE_MENU:
        fsrv_BLE_SetCloseConnection(FALSE);
        fmnu_Activate(AUIM_MNU_INDEX_PAIRING_CLOSE_MENU);
    break;

    /**********************************************************/
    /*                                                        */
    /*        P E R F O R M A N C E      M E N U              */
    /*                                                        */
    /**********************************************************/
    case AUIM_DISPLAY_PERFORMANCE_MENU: {

        send_stpc_packet(); app_log("UI PERFORMANCE_MENU: send_stpc_packet\r\n");

        fsrv_DS_SetMeasurementStatus(MEAS_START);
        fsrv_DS_SetMeasurementMode(MODE_PERFORMANCE);

        fsrv_DS_SetValidIndentations(DEFAULT_VALID_MEASUREMENTS);

        fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_MENU);
        find_DisplayIndicator(FIND_ID_PERFORMANCE);

    } break;

    case AUIM_DISPLAY_PERFORMANCE_CONTINUE_MENU: {

       if (fsrv_DS_GetMeasurementStatus() == MEAS_START) {
          fsrv_start_performance(); app_log("UI fsrv_start_performance\r\n");
          send_on_start_press_performance_packet(); app_log("UI PERFORMANCE_CONTINUE_MENU: send_on_start_press_performance_packet\r\n");
       }

       if (fsrv_DS_GetMeasurementStatus() == MEAS_INCOMPLETE) {
          fsrv_resume_performance(); app_log("UI fsrv_resume_performance\r\n");
       }

       fsrv_DS_SetMeasurementStatus(MEAS_IN_PROGRESS);
       fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_CONTINUE_MENU);

    } break;

    case AUIM_DISPLAY_PERFORMANCE_CANCELED_MENU:
       send_stop_packet();   app_log("UI PERFORMANCE_CANCELED_MENU: send_stop_packet\r\n");
       fsrv_DS_SetMeasurementStatus(MEAS_IDLE);
       fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_CANCELED_MENU);
    break;

    case AUIM_DISPLAY_PERFORMANCE_COMPLETE_MENU: {
       if (fsrv_is_session_performance_bad() == TRUE) {
           fsrv_send_fail_session_packet();    app_log("UI PERFORMANCE_COMPLETE_MENU: send_fail_session_packet\r\n");
       }

       fsrv_DS_SetMeasurementStatus(MEAS_COMPLETE);
       fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_COMPLETE_MENU);
    } break;

    case AUIM_DISPLAY_PERFORMANCE_TERMINATE_MENU:

        fsrv_pause_performance(); app_log("UI fsrv_pause_performance\r\n");
        fsrv_DS_SetMeasurementStatus(MEAS_INCOMPLETE);
        fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_TERMINATE_MENU);
        find_DisplayIndicator(FIND_ID_TERMINATE);
    break;

    case AUIM_DISPLAY_PERFORMANCE_RESULT_MENU:

       fmnu_Activate(AUIM_MNU_INDEX_PERFORMANCE_RESULT_MENU);
       fsrv_send_final_bmsi_packet();   app_log("UI PERFORMANCE_RESULT_MENU: send_final_bmsi_packet\r\n");
       fsrv_stop_performance(); app_log("UI fsrv_stop_performance\r\n");

    break;

     /**********************************************************/
     /*                                                        */
     /*        R E F E R E N C E         M E N U               */
     /*                                                        */
     /**********************************************************/
    case AUIM_DISPLAY_REFERENCE_MENU: {

         send_strm_packet(); app_log("UI REFERENCE_MENU: send_strm_packet\r\n");
         fsrv_switch_to_reference_submode();

         fsrv_DS_SetMeasurementStatus(MEAS_START);
         fsrv_DS_SetMeasurementMode(MODE_REFERENCE);
         fsrv_DS_SetValidIndentations(DEFAULT_VALID_MEASUREMENTS);

         fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_MENU);
         find_DisplayIndicator(FIND_ID_REFERENCE);

    } break;

    case AUIM_DISPLAY_REFERENCE_CONTINUE_MENU: {

       if ( fsrv_DS_GetMeasurementStatus() == MEAS_COMPLETE) {
           fsrv_DS_SetMeasurementStatus(MEAS_START);
           fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_MENU);
           find_DisplayIndicator(FIND_ID_REFERENCE);
       } else {
         if (    ( fsrv_DS_GetMeasurementStatus() == MEAS_START )
              || ( fsrv_DS_GetMeasurementStatus() == MEAS_UNSTABLE ) ) {
             send_on_start_press_performance_reference_packet(); app_log("UI REFERENCE_CONTINUE_MENU: send_on_start_press_performance_reference_packet\r\n");
             fsrv_switch_to_reference_state(); app_log("fsrv_switch_to_reference_state\r\n");
         }

         fsrv_resume_reference(); app_log("UI REFERENCE_MENU: fsrv_resume_reference\r\n");
         fsrv_DS_SetMeasurementStatus(MEAS_IN_PROGRESS);
         fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_CONTINUE_MENU);
       }

    } break;

    case AUIM_DISPLAY_REFERENCE_CANCELED_MENU: {

       Bool is_performance_mode = fsrv_is_performance_mode_state();

       if (is_performance_mode == FALSE) { //IF is current  PATIENT MODE
           fsrv_calculate_aproximate_bmsi();
       }

       send_stop_packet(); app_log("UI REFERENCE_CANCELED_MENU: send_stop_packet\r\n");

       fsrv_DS_SetMeasurementStatus(MEAS_IDLE);

       if (is_performance_mode == TRUE) {
          fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_PERFORMANCE_CANCELED_MENU);
       } else {
          fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_PATIENT_CANCELED_MENU);
       }
    } break;

    case AUIM_DISPLAY_REFERENCE_COMPLETE_MENU: {

       if (fsrv_is_session_reference_bad() == TRUE) {
         fsrv_send_fail_session_packet(); app_log("UI REFERENCE_COMPLETE_MENU: send_fail_session_packet\r\n");
       } else {
         fsrv_DS_SetMeasurementStatus(MEAS_COMPLETE);
         fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_COMPLETE_MENU);
       }

    } break;

    case AUIM_DISPLAY_REFERENCE_TERMINATE_MENU: {

       fsrv_pause_reference(); app_log("UI fsrv_pause_reference\r\n");

       if (fsrv_DS_GetMeasurementStatus() != MEAS_START) {
           fsrv_DS_SetMeasurementStatus(MEAS_INCOMPLETE);
       } else{
           fsrv_DS_SetMeasurementStatus(MEAS_COMPLETE);
       }

       fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_TERMINATE_MENU);
       find_DisplayIndicator(FIND_ID_TERMINATE);
    } break;

    case  AUIM_DISPLAY_REFERENCE_UNSTABLE_REPEAT_MENU: {

        Bool is_performance_mode = fsrv_is_performance_mode_state();
        fsrv_pause_reference(); app_log("UI fsrv_pause_reference\r\n");

        fsrv_DS_SetMeasurementStatus(MEAS_UNSTABLE);
        fsrv_DS_SetMeasurementMode(MODE_REFERENCE);
        fsrv_DS_SetValidIndentations(DEFAULT_VALID_MEASUREMENTS);

        fsrv_send_fail_session_packet(); app_log("UI UNSTABLE_MENU: send_fail_session_packet\r\n");

        if (is_performance_mode == TRUE) {
           fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_PERFORMANCE_UNSTABLE_MENU);
        } else {
           fmnu_Activate(AUIM_MNU_INDEX_REFERENCE_PATIENT_UNSTABLE_MENU);
        }
        find_DisplayIndicator(FIND_ID_UNSTABLE);
    } break;

  /**********************************************************/
  /*                                                        */
  /*        P A T I E N T        M E N U                    */
  /*                                                        */
  /**********************************************************/
    case AUIM_DISPLAY_PATIENT_MENU: {

      send_stpm_packet();  app_log("UI PATIENT_MENU: send_stpm_packet\r\n");

      fsrv_DS_SetMeasurementStatus(MEAS_START);
      fsrv_DS_SetMeasurementMode(MODE_PATIENT);

      fsrv_DS_SetValidIndentations(DEFAULT_VALID_MEASUREMENTS);
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_MENU);
      find_DisplayIndicator(FIND_ID_PATIENT);

    } break;

    case AUIM_DISPLAY_PATIENT_CONTINUE_MENU: {

      if (fsrv_DS_GetMeasurementStatus() == MEAS_START) {
        fsrv_start_patient(); app_log("UI fsrv_start_patient\r\n");
        send_on_start_press_patient_packet();  app_log("UI PATIENT_CONTINUE_MENU: send_on_start_press_patient_packet\r\n");
      }
      if (fsrv_DS_GetMeasurementStatus() == MEAS_INCOMPLETE) {
        fsrv_resume_patient(); app_log("UI fsrv_resume_patient\r\n");
      }

      fsrv_DS_SetMeasurementStatus(MEAS_IN_PROGRESS);
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_CONTINUE_MENU);

    } break;

    case AUIM_DISPLAY_PATIENT_CANCELED_MENU:

      //fsrv_stop_patient(); app_log("UI fsrv_stop_patient\r\n");
      fsrv_pause_patient(); app_log("UI fsrv_pause_patient\r\n");

      send_stop_packet();  app_log("UI PATIENT_CANCEL_MENU: send_stop_packet\r\n");
      fsrv_DS_SetMeasurementStatus(MEAS_IDLE);
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_CANCELED_MENU);
    break;

    case AUIM_DISPLAY_PATIENT_COMPLETE_MENU:

      if (fsrv_is_session_patient_bad() == TRUE) {
        fsrv_send_fail_session_packet();  app_log("UI PATIENT_COMPLETE_MENU: send_fail_session_packet\r\n");
      }

      fsrv_DS_SetMeasurementStatus(MEAS_COMPLETE);
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_COMPLETE_MENU);
    break;

    case AUIM_DISPLAY_PATIENT_TERMINATE_MENU:

      fsrv_pause_patient(); app_log("UI fsrv_pause_patient\r\n");
      fsrv_DS_SetMeasurementStatus(MEAS_INCOMPLETE);
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_TERMINATE_MENU);
      find_DisplayIndicator(FIND_ID_TERMINATE);
    break;

    case AUIM_DISPLAY_PATIENT_RESULT_MENU:

      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_RESULT_MENU);
      if (fsrv_DS_GetMeasurementStatus() == MEAS_UNSTABLE) {
          fsrv_calculate_aproximate_bmsi();
      }

      fsrv_send_final_bmsi_packet();   app_log("UI PATIENT_RES_MENU: send_final_bmsi_packet\r\n");
      fsrv_stop_patient(); app_log("UI fsrv_stop_patient\r\n");
    break;

    case AUIM_DISPLAY_PATIENT_FAILED_MENU:
      fsrv_send_final_bmsi_packet_failed();    app_log("UI PATIENT_FAIL_MENU: send_final_bmsi_packe_failedt\r\n");
      fsrv_stop_patient(); app_log("UI fsrv_stop_patient\r\n");
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_FAILED_MENU);
    break;

    case AUIM_DISPLAY_PATIENT_VALIDATING_MENU: {
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_VALIDATING_MENU);
    }break;

    case AUIM_DISPLAY_PATIENT_VALIDATING_RESULT_MENU: {
      fmnu_Activate(AUIM_MNU_INDEX_PATIENT_VALIDATING_RESULT_MENU);

      if ( fsrv_DS_GetTipIdStatus()  == TIP_ID_VALID ) {
          find_DisplayIndicator(FIND_ID_TIP_ID_VALID);
      } if ( fsrv_DS_GetTipIdStatus() == TIP_ID_INVALID ) {
          find_DisplayIndicator(FIND_ID_TIP_ID_INVALID);
      } else if ( fsrv_DS_GetTipIdStatus()  == TIP_ID_USED ) {
          find_DisplayIndicator(FIND_ID_TIP_ID_USED);
      }

    } break;

    /**********************************************************/
    /*                                                        */
    /*       C H A R G E   B A T T E R Y    M E N U           */
    /*                                                        */
    /**********************************************************/
    case AUIM_DISPLAY_CHARGE_BATTERY_MENU: {
        fmnu_Activate(AUIM_MNU_INDEX_CHARGE_BATTERY_MENU);
    }break;


    /**********************************************************/
    /*                                                        */
    /*       A C T I O N       I T E M S                      */
    /*                                                        */
    /**********************************************************/

    case AUIM_ACTION_CONFIGURATION_NEXT_FIELD:
        result = AU_KEY_NEXT;
    break;

    case AUIM_ACTION_CONFIGURATION_CANCELED:
        if (   (fsrv_DS_GetBatStatus() != BAT_CRITICAL)
            && (fsrv_DS_GetBatteryLevel() >= AEVS_BAT_LOW_10_BOTTOM_BORDER ) )
          result =  AU_KEY_CLOSE;
        else
          result = AU_KEY_CANCEL;
    break;

    case AUIM_ACTION_SEND_STPM_PACKET:
        send_stop_packet();  app_log("UI ACTION PATIENT OR PERFORM: send_stop_packet\r\n");
        result = AU_KEY_CLOSE;
    break;

    case AUIM_NO_ACTION_FUNCTION :      // Fall through
    default: {
      byte_value = (Byte) value;
      if (byte_value == 0) {
          result = AU_KEY_PROCESSED;
      }
    } break;
  }

  return ((osdFieldValue) result);
}


