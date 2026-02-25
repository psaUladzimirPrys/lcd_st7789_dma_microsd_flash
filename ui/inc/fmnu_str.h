/*������������ ���� */
#ifndef _HFMNU_STR_H
#define _HFMNU_STR_H

#define FMNU_NONE_CHAR		   0x20
#define FMNU_CHAR_SEPARATOR  0xB5
#define FMNU_CHAR_CURSOR     0xB6

enum {

FMNU_Menu = 0,

FMNU_Picture,          
FMNU_Brightness,
FMNU_Saturation,

Main_Lang_menu_LAST_STRING

};

enum {

	  FMNU_TEXNO_MENU_CONSTRUCT_PROMPT = Main_Lang_menu_LAST_STRING + 1,
	  FMNU_Texno, 
	  FMNU_Geometry,
	  FMNU_Adjustment,
	  fmnu_str_Texno_Menu_LAST_STRING

};

enum{

   FMNU_LIST_ITEMS_GOOD
  ,FMNU_LIST_ITEMS_BAD

  ,FMNU_LIST_ITEMS_ERROR
	,FMNU_LIST_ITEMS_LAST_STRING

};

 

 

enum{

   FMNU_PREFIX_NONE = 0,
   FMNU_PREFIX_ID_Minus,
   FMNU_PREFIX_ID_Plus,
   fmnu_str_PrefixLAST_STRING

    }; 

enum{

   FMNU_SUFFIX_NONE = 0,
   FMNU_SUFFIX_ID_db,
   fmnu_str_SuffixLAST_STRING

};

extern  Byte const fmnu_str_List[FMNU_LIST_ITEMS_LAST_STRING];

extern  Byte * fmnu_str_GetVerTexnoMenu(void);

#endif


