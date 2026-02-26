/*=======================================================================*/

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "stddef.h"
#include "hglobal.h"

#include "aukh.h"
#include "auph.h"
#include "fuim.h"
#include "auim_api.h"
#include "auim_ind.h"


#include "find_api.h"
#include "fuim_obs.h"
//#include "fuim_trs.h"

/*=======================================================================*/
/* G L O B A L   R E F E R E N C E S                                     */
/*=======================================================================*/

/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/

#define EMPTY_INDICATOR          0xFF  /*  ��� ����������	  */

#define NR_OF_DOUBLE_INDICATORS  0xFF  /*  ������� ��������� */
 	 

#define INDICATOR_TIME_OUT       0x03  /*  Time out in seconds */
#define INDICATOR_NO_TIME_OUT    0x00  /*  No Time out         */

 
 
/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                     */
/*=======================================================================*/
/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/

static Byte  active_indicators[FUIM_MAX_INDICATORS];
static Byte  find_IndicatorFocus;


/*=======================================================================*/
/*  The following array is used to "map" OSD ID's to indicator ID's.     */
/*  Note that special actions must be done for the volume OSD, since     */
/*  this OSD is made up of two indicators   
                            */

const Byte indicator_ids[] =   /*             */
{/* Index in array  auim_OsdIndicator/<- = ->/       */

    AUIM_INDEX_BATTERY_INDICATOR      /* FIND_ID_BATTERY       Volume (String)   */
    ,AUIM_INDEX_BLE_INDICATOR          /* FIND_ID_BLE       Volume (String)       */
    ,AUIM_INDEX_SYNC_INDICATOR         /* FIND_ID_SYNC       Volume (String)      */

 //   ,NR_OF_DOUBLE_INDICATORS

};


static Bool RestoreAllIndicators;
static Bool findDisplayTemporaryProgNumber;
static Bool findDisplayTemporaryClock;

/*=======================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                  */
/*=======================================================================*/


 void CreateIndicator(find_id_enum indicator);
 Bool find_IsRemoveIndicator(fuimIndicatorStruct  *indicator_data_ptr );
void find_IndicatorAction(fuimIndicatorStruct  *indicator_data_ptr );

 void find_ProcessIndicatorAction(cmdKeyNumber action,	fuimFieldStruct  *field_data_ptr );
 void RemoveEmptyIndicator(find_id_enum indicator);
/*=======================================================================*/
/*=======================================================================*/
void find_Init(void)
{
   findDisplayTemporaryClock = FALSE;

}

/*=======================================================================*/
/*=======================================================================*/

void find_TurnOn(void)
{
   Byte  i;
   
   fuim_InitIndicators();
   for (i = 0; i < FUIM_MAX_INDICATORS; i++ )
   {
        active_indicators[i] = EMPTY_INDICATOR;
 
   }
 
   find_IndicatorFocus = TRUE;
   RestoreAllIndicators = FALSE;

}

void find_ChangeDisplayTemporaryClock(void)
{
    findDisplayTemporaryClock = ~findDisplayTemporaryClock;
 
}
/*=======================================================================*/
/*=======================================================================*/

Bool find_GetDisplayTemporaryClock(void)
{
 return  findDisplayTemporaryClock;	
}

/*=======================================================================*/
/*=======================================================================*/

void find_ChangeDisplayTemporaryProgNumber(void)
{
    findDisplayTemporaryProgNumber = ~findDisplayTemporaryProgNumber;
//	rpms_Set1Bit( FPMS_TV_SETUP , FIND_CFG_DISPLAY_PROG_NUMBER  , findDisplayTemporaryProgNumber  );
}




/*=======================================================================*/
/*************************************************************************
   @func   This displays an indicator on the screen.

   @parm   Indicator: Indicator that must be displayed

   @comm   Pre condition  : none

   @comm   Post condition : none

**************************************************************************/  
void find_RestoreIndicators(void)
{
  
   find_DisplayIndicator(FIND_ID_BATTERY);
   
   if ( findDisplayTemporaryClock )
   {
   	find_DisplayIndicator(FIND_ID_CLOCK);
   }

}   
/*=======================================================================*/
void find_SetRestoreAllIndicators(Bool NewVal)
{
  RestoreAllIndicators = NewVal;	
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/************************************************************************
   @func   This displays an indicator on the screen.
   @parm   Indicator: Indicator that must be displayed

***********************************************************************/  
void find_RemoveAllIndicators(void)
{
   Byte  i;
   
   for (i = 0; i < FUIM_MAX_INDICATORS; i++)
   {
      if (active_indicators[i] != EMPTY_INDICATOR)
      {
         find_RemoveIndicator( active_indicators[i] );
      }
   }
#if 0
   for( i = 0; i < FUIM_MAX_NR_OF_ROWS ;i++)
  
  {
   plt_CCSetPosition(i, 0 );
   plt_CCDrawChar(' '); 
  }
#endif
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Bool find_IsIndicatorDisplayed(Byte indicator)
{
   Byte  active;
   
   for (active = 0; active < FUIM_MAX_INDICATORS; active++)
   {
      if (active_indicators[active] == indicator)
      {
         return TRUE;
      }
   }
   return FALSE;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_DisplayIndicator(find_id_enum indicator)
{

   
   if (auph_GetState() == AU_DIRECT_STATE)
   {

     Byte  active;

      if (!find_IsIndicatorDisplayed(indicator))
      {
         active = 0;

         while ((active_indicators[active] != EMPTY_INDICATOR) && (active < FUIM_MAX_INDICATORS))
         {
            active++;
         }

         if ((active < FUIM_MAX_INDICATORS) && (active_indicators[active] == EMPTY_INDICATOR))
         {
            CreateIndicator(indicator);
            active_indicators[active] = indicator;
         }
      }
      else find_UpdateIndicator(indicator);

   }

}

/*=======================================================================*/
/*    L O C A L   F U N C T I O N S                                      */
/*=======================================================================*/
 
/*************************************************************************
   @    Indicator: Indicator that must be displayed

***************************************************************************/  
void CreateIndicator(find_id_enum indicator)
{

   if (indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS)
   {
      fuim_ConstructIndicator((fuimIndicatorStruct   *)&auim_OsdIndicator[indicator_ids[indicator]]);
   }

}   

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_Update(void)
{

 if (auph_GetState() == AU_DIRECT_STATE)
{ 
  Byte  i, indicator;
   
   for (i = 0; i < FUIM_MAX_INDICATORS; i++)
   {
      indicator = active_indicators[i];
      if (indicator != EMPTY_INDICATOR)
      { 
      	
      RemoveEmptyIndicator(indicator);

      }
   }
  
   if(RestoreAllIndicators == TRUE)
   {
     find_RestoreIndicators();
     RestoreAllIndicators = FALSE;
   } 

 }
} 
/*=======================================================================*/
/*************************************************************************
   @func   This displays an indicator on the screen.

   @parm   Indicator: Indicator that must be displayed

**************************************************************************/  
void find_RemoveIndicator(find_id_enum indicator)
{
   Byte  active;
osdDialogHandle  handle;


   if (find_IsIndicatorDisplayed(indicator))
   {

      active = 0;

      while (active_indicators[active] != indicator)
      {
      active++;
      }

	   if (indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS)
	   { 
       handle = fuim_GetIndicatorHandle((fuimIndicatorStruct   *)&auim_OsdIndicator[indicator_ids[indicator]]);
	     fuim_DestroyIndicator(handle);
	   }
/*	   else
	   {
			switch(indicator)
	  		{
	 
	        case FIND_ID_SOURCE:	{
	
            fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_NUMBER_INDICATOR]));
            fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_STRING_INDICATOR]));
	                             
									} break;
	         case FIND_ID_MENU:	{
								fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR]));
								fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_LEFT]));
								fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_RIGHT]));
								}break;
  	       case FIND_ID_NO_SIGNALS:
								{
							    fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_NO_SIGNAL_INDICATOR]));
							    fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_NO_SIGNAL_TIME_INDICATOR]));
								}break;
	
		   case FIND_ID_NO_PROGRAM:
								{
		    fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_1]));
		    fuim_DestroyIndicator(fuim_GetIndicatorHandle(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_2]));
	 							 }break;
	
		 	 }

	    }*/
 
      active_indicators[active] = EMPTY_INDICATOR;
   }

}   

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_UpdateIndicator(find_id_enum indicator)
{

   fuimIndicatorStruct  *indicator_data_ptr;
 
   if (indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS)
   {
      indicator_data_ptr = (fuimIndicatorStruct  *)&auim_OsdIndicator[indicator_ids[indicator]];
      find_IndicatorAction(indicator_data_ptr );
    }
/*
   else 
   {
    switch(indicator)
        {
         case FIND_ID_SOURCE:{

 find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_NUMBER_INDICATOR] );
 find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_STRING_INDICATOR]  );
 
							}break;
         case FIND_ID_MENU:{
 find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_LEFT]  );
 find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR]  );
 find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_RIGHT]  );

							}break;     
   case FIND_ID_NO_PROGRAM:{
   	
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_1]);
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_2]);
    
	 		}break;
							                    
         default: break;

        }

   }
*/

} 
 
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_IndicatorAction(fuimIndicatorStruct *indicator_data_ptr )
{
//	   fuimFieldStruct  * field_data_ptr;
			osdDialogHandle   handle;
			
    handle = fuim_GetIndicatorHandle(indicator_data_ptr);

    if ((handle != 0 )&&( handle < FUIM_MAX_INDICATORS))
     {  
//       field_data_ptr = indicator_data_ptr->Field;
      // find_ProcessIndicatorAction(aukh_GetCurrentCommand(),field_data_ptr);
       fuim_UpdateIndicator (handle, TRUE);
     }
 
} 

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Bool find_IsRemoveIndicator(fuimIndicatorStruct  *  indicator_data_ptr )
{
  osdDialogHandle  handle;

  handle = fuim_GetIndicatorHandle(indicator_data_ptr);

  if ( (handle != 0) && (handle < FUIM_MAX_INDICATORS) )
  {
      return FALSE;
  }

  return TRUE;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void RemoveEmptyIndicator(find_id_enum indicator)
 {
 
 fuimIndicatorStruct *indicator_data_ptr;
 Bool PassRemove = TRUE; 
 Byte   active;
   if (indicator_ids[ indicator ] < NR_OF_DOUBLE_INDICATORS)
   {
        indicator_data_ptr = (fuimIndicatorStruct *)&auim_OsdIndicator[indicator_ids[indicator ]];
               PassRemove &= find_IsRemoveIndicator(indicator_data_ptr  );
    }
   /*
      else
      {
       switch(indicator)
           {
            case FIND_ID_SOURCE:{

    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_NUMBER_INDICATOR] );
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_PROGRAM_STRING_INDICATOR]  );

                 }break;
            case FIND_ID_MENU:{
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_LEFT]  );
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR]  );
    find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_MENU_INDICATOR_RIGHT]  );

                 }break;
      case FIND_ID_NO_PROGRAM:{

       find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_1]);
       find_IndicatorAction(&auim_OsdIndicator[AUIM_INDEX_NO_PROGRAM_INDICATOR_2]);

         }break;

            default: break;

           }

      }
   */


if(PassRemove == TRUE)
 {

   			if (find_IsIndicatorDisplayed(indicator))
  			 {
     		 active = 0;
 		        while (active_indicators[active] != indicator)
		        {
      		    active++;
   		        }
             active_indicators[active] = EMPTY_INDICATOR;
             } 	
  }
  
}    

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_ProcessIndicatorAction(cmdKeyNumber action ,	fuimFieldStruct  *   field_data_ptr )
{

	 fuimDialogNavigation  *   ActionPtr;
 
 
     	Byte   new_action;
 
 
		if (field_data_ptr != NULL)
    {
		    ActionPtr = field_data_ptr -> ToDoWithKey;

			if( ActionPtr != NULL )
			{
 			   do
 			   {
			   	if( ActionPtr->Action == action )
					{
								
			   	   		new_action = fuim_ActionHandler (ActionPtr->DialogFunction, action);

                if ( new_action != AU_KEY_PROCESSED )
                {
                  action = new_action;
                 }
                 else
                 {
                      action = AU_KEY_PROCESSED;
                 }
			   	  }

				   if( ActionPtr->Action != AU_KEY_INVALID ) ActionPtr++;
 			   }
 			   while( ActionPtr->Action != AU_KEY_INVALID );
			}
		}
 			
///////////////////////////////////////////////////////////////////////////////////

/*
	switch( action )
	{
  
		case AU_KEY_LEFT:  
		case AU_KEY_RIGHT:
						   if( field_data_ptr -> Type == FUIM_FIELDTYPE_SLIDER )
						   {
							    fuim_Transformer ( field_data_ptr -> ChangeFunction, (action==AU_KEY_LEFT) ? RGEN_CHANGE_DOWN: RGEN_CHANGE_UP);
 
						   }

 						   action = AU_KEY_PROCESSED ;
						   break;
  }
*/
///////////////////////////////////////////////////////////////////////////////////

 

 
} 
 
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Byte find_GetIndicatorFocus(void)
{
return find_IndicatorFocus;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_SetIndicatorFocus( Byte IndicatorFocus )
{
  find_IndicatorFocus = IndicatorFocus;
}
 

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void find_ToggleStatusIndicator(void)
{
  Byte  handle = 0;

  find_DisplayIndicator(FIND_ID_BATTERY);

/*if ( (findDisplayTemporaryClock == FALSE)&&
     (find_IsIndicatorDisplayed(FIND_ID_CLOCK) == FALSE))
  {


     find_DisplayIndicator(FIND_ID_CLOCK);
     handle = fuim_GetIndicatorHandle(&auim_OsdIndicator[indicator_ids[FIND_ID_CLOCK]]);
     fuim_SetIndicatorTimeOut(handle,6);

  }

  if((findDisplayTemporaryProgNumber == FALSE)&&
     (find_IsIndicatorDisplayed(FIND_ID_SOURCE) == FALSE))
  {
      find_DisplayIndicator( FIND_ID_SOURCE);
  }*/

  handle = handle;

}
