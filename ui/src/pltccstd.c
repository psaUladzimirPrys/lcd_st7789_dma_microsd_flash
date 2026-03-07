/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <global.h>
#include <pltccstd.h>

/*=======================================================================*/
/*        G L O B A L    D E F I N I T I O N S                           */
/*=======================================================================*/
/*=======================================================================*/
/*        L O C A L   D A T A   D E C L A R A T I O N S                  */
/*=======================================================================*/
static Word  CCRows;             //Current position Row and Column
static Word  CCColumn;
 
static Word  CCMaxRows;          //Мах values of Rows and Columns
static Word  CCMaxColumns;

static Word  CCBackgroundColour;
static Word  ССForegroundColour;

static Byte  CCRowSize;          //Height
/*==========================================================================*/
/* L O C A L        F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
/*==========================================================================*/
/* L O C A L        F U N C T I O N                                         */
/*==========================================================================*/
/*==========================================================================*/
/* G L O B A L      F U N C T I O N                                         */
/*==========================================================================*/
/*****************************************************************************
*
* NAME: pltstd_CCInit
*
* Returns:  void
*
* Parameter    Flow    Description
* --------------------------------------------------------------------------
* Length        IN      Required number of columns
* Height        IN      Required number of rows
******************************************************************************/
void plt_CCInit(Word Length, Word Height)
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
void plt_CCSetPosition(Word Row, Word Column)
{
  CCRows = Row;       // Current row value
  CCColumn = Column;  // Current column value
}

/*******************************************************************************
*@begin
* NAME: plt_CCGetPosition
*
* Gets the display position for data writes.
*
* Returns :  void
*
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Row           OUT     Current Row
* Column        OUT     Current Column
*@end
********************************************************************************/
void plt_CCGetPosition(Word *Row, Word *Column)
{
	* Row    = CCRows;
	* Column = CCColumn;
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
*@end
**********************************************************************************/ 
void plt_CCSetBackgroundColour(Word BackgroundColour)
{
  CCBackgroundColour = BackgroundColour;
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
*@end
********************************************************************************/ 
void plt_CCSetForegroundColour(Word ForegroundColour)
{
  ССForegroundColour = ForegroundColour;
}

/******************************************************************************* 
*
*
*
*
********************************************************************************/ 
void plt_CCGetForeGndBackGndColours(Word *ForeGndColour, Word *BackGndColour)
{
  * ForeGndColour = ССForegroundColour;
  * BackGndColour = CCBackgroundColour;
}

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

/*******************************************************************************
*
*
*
*
********************************************************************************/
void plt_CCSetRowSize(Byte Size)
{
  CCRowSize = Size;
}

/*******************************************************************************
*
*
*
*
********************************************************************************/
Byte plt_CCGetRowSize(void)
{
  return CCRowSize;
}
