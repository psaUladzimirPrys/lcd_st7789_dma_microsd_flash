#ifndef _HFMNU_STR_H
#define _HFMNU_STR_H

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/


/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define FMNU_NONE_CHAR		   0x20
#define FMNU_CHAR_SEPARATOR  0xB5
#define FMNU_CHAR_CURSOR     0xB6


enum {

   FMNU_LIST_ITEMS_GOOD
  ,FMNU_LIST_ITEMS_BAD

  ,FMNU_LIST_ITEMS_ERROR
  ,FMNU_LIST_ITEMS_LAST_STRING

};
 

enum {

   FMNU_PREFIX_NONE = 0,
   FMNU_PREFIX_ID_Minus,
   FMNU_PREFIX_ID_Plus,
   fmnu_str_PrefixLAST_STRING

};

enum {

   FMNU_SUFFIX_NONE = 0,
   FMNU_SUFFIX_ID_db,
   fmnu_str_SuffixLAST_STRING

};

extern const Byte fmnu_str_List[FMNU_LIST_ITEMS_LAST_STRING];
extern const Byte fmnu_str_Prefix[fmnu_str_PrefixLAST_STRING];
extern const Byte fmnu_str_Suffix[fmnu_str_SuffixLAST_STRING];

 

#endif


