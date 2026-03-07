
#ifndef  _HCCSTD_H
#define  _HCCSTD_H

/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include "global.h"

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define BEGIN_DISP_X_CC   4
#define SIZE_DISP_X_CC   48
#define BEGIN_DISP_Y_CC  42

#define BEGIN_LINE_SCROLL_AREA  4
#define	HEIGHT_SCROLL_AREA      5 
#define START_SCROLL_LINE       3
#define STOP_SCROLL_LINE		   11
#define TOP_SCROLL_LINE         3
#define FIRST_SCROLL_LINE 		  3
 
/******************************************************************************
* НАЗВАНИЕ: pltstd_CCInit
* Initializes the display object before use.
*
* Return :  void
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
* Length       IN       Required number of columns
* Height       IN       Required number of rows
********************************/
void plt_CCInit(Word Length, Word Height);

/*******************************************************************************
* NAME: plt_CCSetPosition
* Sets the display position for data recording
*
* Returns :  void
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
-
* Row           IN     The row at which the OSD information begins.
* Column        IN     The column at which the OSD information begins.
********************************/
void plt_CCSetPosition(Word Row, Word Column);


/*******************************************************************************
* NAME: plt_CCDrawChar
* Displays the symbol on the screen
* Returns :  void
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* Character     IN      Symbol recorded for display
*
********************************/
void plt_CCDrawChar(char Character);

/********************************************************************************
* NAME: plt_CCSetBackgroundColour
*
* Sets the required background color of characters which are to be written
*
* Returns :  void
*
* Parameter             Flow    Description
* -------------------------------------------------------------------------------
* BackgroundColour      IN      Color to be set as the background
*
 *@end
********************************/
void plt_CCSetBackgroundColour(Word BackgroundColour);

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
*@end
********************************/
void plt_CCSetForegroundColour(Word ForegroundColour);
 
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
void plt_CCGetPosition(Word * Row, Word * Column)  ;

/********************************************************************************
*@begin
* NAME: plt_CCSetRowSize
*
*       Set the row size for the given row by ensuring a serial mode 1 attribute
*       is written to char position 1. This may cause loss of information if
*       valid serial mode 0 data or character (parallel) data is already in this
*       location.
*
*       ** THIS FUNCTION CAN OVERWRITE VALID DATA **
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Size         In      The required size from:
*                      0 single width single height
*                      1 double width single height
*                      2 single width double height
*                      3 double width double height
*
*@end
**************************************************************************/
void plt_CCSetRowSize(Byte Size);

Byte plt_CCGetRowSize(void);

void plt_CCGetForeGndBackGndColours(Word * ForeGndColour, Word * BackGndColour);
 
  
#endif /* Do not add any thing below this line */


