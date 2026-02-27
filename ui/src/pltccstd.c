/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "hglobal.h"
#include "ccstd.h"

/*============================================ ==================*/
/*        G L O B A L   D E F I N I T I O N S                     */
/*============================================= =================*/ 



/*==========================================================================*/
/*        L O C A L   D A T A   D E C L A R A T I O N S                     */
/*==========================================================================*/
static Byte  CCRows;             //Current position Row and Column
static Byte  CCColumn; 
 
 
static Byte  CCMaxRows;          //Мах values of Rows and Columns
static Byte  CCMaxColumns;

/*==========================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                        */
/*==========================================================================*/

/*==========================================================================*/
/* L O C A L   F U N C T I O N                                              */
/*==========================================================================*/

/*==========================================================================*/
/* G L O B A L      F U N C T I O N                                         */
/*==========================================================================*/

/*****************************************************************************
*
* NAME: pltstd_CCInit
*  
*
* Returns:  void
*
* Parameter    Flow    Description
* --------------------------------------------------------------------------
* Length        IN      Required number of columns
* Height        IN      Required number of rows
******************************************************************************/

void pltstd_CCInit(Byte Length, Byte Height)
{
  CCMaxColumns = Length; // Store the requested number of columns in memory
  CCMaxRows    = Height; //  and rows
}

/*******************************************************************************
* NAME: plt_CCSetPosition
* Sets the display position for data recording
*
* Returns :  void
*
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
*
* Row           IN      Start row for OSD information recording.
* Column        IN      Start column for OSD information recording.
********************************************************************************/

void plt_CCSetPosition(Byte Row, Byte Column) 
{
  CCRows = Row;      // Current row value
  CCColumn = Column;  // Current column value
}
 

/*******************************************************************************
*
*@begin
* NAME: plt_CCGetPosition
*
* Gets the display position for data writes.
*
* Returns :  void
*
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
-
* Row           OUT     Current Row
* Column        OUT     Current Column
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
-
*
* Additional information:
*
*@end
********************************************************************************/
void plt_CCGetPosition(Byte   *Row, Byte   *Column) 
{
	* Row    = CCRows;
	* Column = CCColumn;
}


/*******************************************************************************
* NAME: plt_CCSetOverline
*
*  
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Mode          IN       
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
********************************/

void plt_CCSetOverline(Bool Mode)
{
Mode  = Mode;
}
/*******************************************************************************
* NAME: plt_CCSetUnderline
*
*  
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Mode          IN       
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
********************************/
void plt_CCSetUnderline(Bool Mode)
{
Mode = Mode;
}


/********************************************************************************
* NAME: plt_CCSetBackgroundColour
*
* Returns :  void
*
* Parameter             Flow    Description
* -------------------------------------------------------------------------------
* BackgroundColour      IN      Color to be set as the background
* SetAt                 IN      Flag to indicate when the attribute should take effect
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*
*
*@end
**********************************************************************************/ 
void  plt_CCSetBackgroundColour(Byte BackgroundColour, Byte SetAt) 
{
 	  BackgroundColour = BackgroundColour&0x0F;

  if( SetAt == TRUE)/* SetAt = 1     */
   {
      BackgroundColour = 0;

   }

 
}
/********************************************************************************
*@begin
* NAME: plt_CCSetForegroundColour
*
* Sets the required foreground color for characters to be written in
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* ForegroundColour      IN      Color to be set as the foreground
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************************************************************/ 

void plt_CCSetForegroundColour(Byte ForegroundColour) 
{
  ForegroundColour = ForegroundColour&0x7;
}

/******************************************************************************* 
*
*
*
*
********************************************************************************/ 
/*
void plt_CCInstForegroundColour(Byte ForegroundColour) 
{
  ForegroundColour = ForegroundColour;
}
*/
/*******************************************************************************
* NAME: plt_CCDrawChar
*
* Returns :  void
*
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Character     IN
*
* Externals    Flow    Usage
*******************************************************************************/

void plt_CCDrawChar(char Character)    
{
  Character = Character;
  CCColumn++;
}
 
/**********************************************************************************
* NAME: plt_CCSetBoxBackground
*
* Turns background boxing on/off
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Mode          IN      Флаг, чтобы указать, боксировал ли фон.
* SetAt         IN      Флаг, чтобы указать, когда признак должен вступить в силу
*
***********************************************************************************/ 
  void  plt_CCSetBoxBackground(Byte Mode, Byte SetAt) 
{

	Mode = Mode&0x01;

  if( SetAt == TRUE )/*    SetAt  */
   {
 Mode = 0;

   }

 
} 

/**********************************************************************************
*
*
***********************************************************************************/
void MY_plt_CCSetPosition(Byte Row, Byte Column)
{
  CCRows = Row&0x3F;     // Текущее значение строк
  CCColumn = Column;    // Текущее значение столбцов
}


