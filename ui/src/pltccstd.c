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

/*--- Global/Static variables ---*/

static Word  CCRows;             /* Current row position */
static Word  CCColumn;           /* Current column position */
 
static Word  CCMaxRows;          /* Max values for rows and columns */
static Word  CCMaxColumns;

static Word  CCBackgroundColour; /* Current background color */
static Word  CCForegroundColour; /* Current foreground color */

static Byte  CCRowSize;          /* Height/row size in pixels/units */
/*==========================================================================*/
/* L O C A L        F U N C T I O N   P R O T O T Y P E S                   */
/*==========================================================================*/
/*==========================================================================*/
/* L O C A L        F U N C T I O N                                         */
/*==========================================================================*/
/*==========================================================================*/
/* G L O B A L      F U N C T I O N                                         */
/*==========================================================================*/
/*=================================================================================
*   Function:    plt_CCInit
*   Description: Initializes the character-cell display driver by setting the
*                maximum allowed rows and columns for subsequent positioning.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Length       IN      Requested number of columns
* Height       IN      Requested number of rows
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCMaxColumns   OUT  Set to Length
* CCMaxRows      OUT  Set to Height
*
* @return void
*
* @note Does not perform bounds checking on Length/Height values.
===================================================================================*/
void plt_CCInit(Word Length, Word Height)
{
  CCMaxColumns = Length; // Store the requested number of columns in memory
  CCMaxRows    = Height; //  and rows
}

/*=================================================================================
*   Function:    plt_CCSetPosition
*   Description: Sets the current cursor position for character-cell rendering.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Row          IN      Target row position
* Column       IN      Target column position
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCRows   OUT  Updated with Row value
* CCColumn OUT  Updated with Column value
*
* @return void
*
* @note No bounds checking against CCMaxRows/CCMaxColumns is performed.
===================================================================================*/
void plt_CCSetPosition(Word Row, Word Column)
{
  CCRows = Row;       // Current row value
  CCColumn = Column;  // Current column value
}

/*=================================================================================
*   Function:    plt_CCGetPosition
*   Description: Retrieves the current cursor position for character-cell rendering.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Row          OUT     Current row position
* Column       OUT     Current column position
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCRows   IN  Source for current row
* CCColumn IN  Source for current column
*
* @return void
*
* @note Both Row and Column pointers must be non-NULL.
===================================================================================*/
void plt_CCGetPosition(Word *Row, Word *Column)
{
  * Row    = CCRows;
  * Column = CCColumn;
}

/*=================================================================================
*   Function:    plt_CCSetBackgroundColour
*   Description: Sets the background color used for subsequent character-cell
*                rendering operations.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* BackgroundColour IN  Color value to set as background
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCBackgroundColour   OUT  Updated with BackgroundColour value
*
* @return void
*
* @note Color value range is platform-dependent (typically ST7789 color format).
===================================================================================*/
void plt_CCSetBackgroundColour(Word BackgroundColour)
{
  CCBackgroundColour = BackgroundColour;
}

/*=================================================================================
*   Function:    plt_CCSetForegroundColour
*   Description: Sets the foreground color used for subsequent character-cell
*                rendering operations.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* ForegroundColour IN  Color value to set as foreground
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCForegroundColour   OUT  Updated with ForegroundColour value
*
* @return void
*
* @note Color value range is platform-dependent (typically ST7789 color format).
===================================================================================*/
void plt_CCSetForegroundColour(Word ForegroundColour)
{
  CCForegroundColour = ForegroundColour;
}

/*=================================================================================
*   Function:    plt_CCGetForeGndBackGndColours
*   Description: Retrieves the currently set foreground and background colors.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* ForeGndColour   OUT  Current foreground color value
* BackGndColour   OUT  Current background color value
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCForegroundColour   IN  Source for foreground color
* CCBackgroundColour   IN  Source for background color
*
* @return void
*
* @note Both pointers must be non-NULL.
===================================================================================*/
void plt_CCGetForeGndBackGndColours(Word *ForeGndColour, Word *BackGndColour)
{
  * ForeGndColour = CCForegroundColour;
  * BackGndColour = CCBackgroundColour;
}


/*=================================================================================
*   Function:    plt_CCSetRowSize
*   Description: Sets the row height (in pixels/units) for subsequent character-cell
*                rendering operations.
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Size         IN      Row height value to set
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCRowSize   OUT  Updated with Size value
*
* @return void
*
* @note Typically callers AND Size with FUIM_ATTRIBUTES_ROW_SIZE mask before calling.
===================================================================================*/
void plt_CCSetRowSize(Byte Size)
{
  CCRowSize = Size;
}

/*=================================================================================
*   Function:    plt_CCGetRowSize
*   Description: Retrieves the currently set row height value.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* CCRowSize   IN  Source for current row size
*
* @return Current row size as Byte
*
* @note Value is typically ANDed with FUIM_ATTRIBUTES_ROW_SIZE by callers.
===================================================================================*/
Byte plt_CCGetRowSize(void)
{
  return CCRowSize;
}
