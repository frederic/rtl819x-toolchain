#include "ui_config.h"
#include "res_ime.h"

#ifdef LANG_BIG5
#include "res_ime_big5_pinyin.c"
#include "res_ime_big5_phonetic.c"
#endif

#ifdef LANG_GB2312
#include "res_ime_gb2312_pinyin.c"
#include "res_ime_gb2312_wubihua.c"
#endif

