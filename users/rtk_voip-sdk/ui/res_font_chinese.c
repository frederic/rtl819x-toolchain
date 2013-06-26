#include "ui_config.h"
#include "res_font.h"

#if defined( LANG_BIG5 ) && defined( LCD_COL2_ORIENTED )
#include "res_font_big5_16x16_col2.c"
#include "res_font_big5_14x14_col2.c"
#endif

#if defined( LANG_GB2312 ) && defined( LCD_COL2_ORIENTED )
#include "res_font_gb2312_16x16_col2.c"
#include "res_font_gb2312_14x14_col2.c"
#endif
