
#ifndef  _HCCSTD_H
#define  _HCCSTD_H

/******************* 
* INCLUDE FILES    *

********************/


#define BEGIN_DISP_X_CC   4
#define SIZE_DISP_X_CC   48
#define BEGIN_DISP_Y_CC  42

#define BEGIN_LINE_SCROLL_AREA  4
#define	HEIGHT_SCROLL_AREA      5 
#define START_SCROLL_LINE       3
#define STOP_SCROLL_LINE		11
#define TOP_SCROLL_LINE         3
#define FIRST_SCROLL_LINE 		3

 


#define BEGIN_DISP_X_TXT  4  //целых символов
#define SIZE_DISP_X_TXT  40  //целых символов
#define BEGIN_DISP_Y_TXT 42  //строк

#define END_DISP_X_TXT (BEGIN_DISP_X_TXT + SIZE_DISP_X_TXT)
 
 
/******************************************************************************
 
* НАЗВАНИЕ: pltstd_CCInit
* Инициализирует  дисплейный обьект перед использованием
*  
*
* Возвращает :  void
*
* Параметр    Flow    Описание
* -------------------------------------------------------------------------------
* Length        IN      Требемое число столбцов
* Height        IN      Требуемое число строк
* PageANotB     IN      Флаг, чтобы указать, предназначен ли OSD для страницы A или
*                                   страницы B дисплея (Действительно для Painter2 только)
*
 
********************************/

extern void pltstd_CCInit(Byte Length, Byte Height)  ;

/*******************************************************************************
* NAME: plt_CCSetPosition
* Устанавливает поицию дисплея для записи данных
*
* Returns :  void
*
* Параметр    Вх/Вых    Описание
* ------------------------------------------------------------------------------
-
* Row           IN      Строка начала записи информации OSD.
* Column        IN      Столбец записи информации OSD.
********************************/

 extern void plt_CCSetPosition(Byte Row, Byte Column)  ;


/*******************************************************************************
* NAME: plt_CCDrawChar
*  Отображает символ на экране
* Returns :  void
*
* Параметр    Вх/Вых    Описание
* ------------------------------------------------------------------------------
* Character     IN      Символ записанный к отображению
*
* Externals    Flow    Usage
********************************/

extern void plt_CCDrawChar(char Character)  ;
 


/********************************************************************************
* NAME: plt_CCSetBackgroundColour
*
* Sets the required background colour of characters which are to be written
*
* Returns :  void
*
* Parameter             Flow    Description
* -------------------------------------------------------------------------------
* BackgroundColour      IN      Colour to be set as the background
* SetAt                 IN      Flag to indicate when the attribute should take effect
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
 *@end
********************************/

extern void plt_CCSetBackgroundColour(Byte BackgroundColour, Byte SetAt)  ;


/********************************************************************************
*@begin
* NAME: plt_CCSetForegroundColour
*
* Sets the required foreground colour for characters to be written in
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* ForegroundColour      IN      Colour to be set as the foreground
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************/

 extern void plt_CCSetForegroundColour(Byte ForegroundColour)  ;

/********************************************************************************
*@begin
* NAME: plt_CCSetBoxBackground
*
* Turns background boxing on/off
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Mode          IN      Flag to indicate if the background should be boxed.
* SetAt         IN      Flag to indicate when the attribute should take effect
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************/
 extern void plt_CCSetBoxBackground(Byte Mode, Byte SetAt)  ;

 

 
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

extern void plt_CCGetPosition(Byte   *Row, Byte   *Column)  ;

/*******************************************************************************
*@begin
* NAME: plt_CCSetItalic
*
* Turns on/off the use of italics for subsequent characters
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
* Mode          IN      Flag to indicate if italics should be enabled or not
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************/

 
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
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************/
//extern void plt_CCSetRowSize(Byte Size )  ;
 

/********************************************************************************
*@begin
* NAME: plt_CCUnsetRowEnd
*
* Unsets the end of row attribute.
*
* Returns :  void
*
* Parameter    Flow    Description
* -------------------------------------------------------------------------------
*
* Externals    Flow    Usage
* -------------------------------------------------------------------------------
*
* Additional information:
*
*@end
********************************/

extern void plt_CCUnsetRowEnd(void)  ;



 
/////////////////////////////////////////////////////////////////////////////////

extern void MY_plt_CCSetPosition(Byte Row, Byte Column)  ;
 
  
#endif /*      Do not add any thing below this line */


