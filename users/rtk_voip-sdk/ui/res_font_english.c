#include "ui_config.h"
#include "res_font.h"

#if CHAR_WIDTH == 5 && CHAR_HEIGHT == 8 && defined( LCD_COL_ORIENTED )
#include "res_font_english_5x8_col.c"
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_COL2_ORIENTED )
#include "res_font_english_8x16_col2.c"
#include "res_font_english_nx16_col2.c"
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_ROW_ORIENTED )
#include "res_font_english_8x16_row.c"
#endif
