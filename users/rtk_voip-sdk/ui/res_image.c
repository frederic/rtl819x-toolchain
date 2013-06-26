#include "ui_config.h"
#include "res_image.h"

#ifndef _TEXT_MODE
  #if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
    #include "res_image_ime.c"
  #endif
#endif
