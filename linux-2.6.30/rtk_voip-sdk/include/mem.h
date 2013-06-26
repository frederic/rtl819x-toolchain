
#ifndef MEM_H
#define MEM_H
#include "rtk_voip.h"

//#define BYTE 	3
//#define HWORD 	2
//#define WORD 	1
//#define DEBUG 	1
//#define PADIR 	0xbd010047
//#define PADAT 	0xbd01004b

#ifdef SUPPORT_PCM_FIFO
#define RX_BUF_SIZE PCM_PERIOD_10MS_SIZE   // 10ms LPCM data -> 160 Bytes
#define TX_BUF_SIZE PCM_PERIOD_10MS_SIZE
#else
#define RX_BUF_SIZE 480   // 480 is enough for 30ms LPCM data
#define TX_BUF_SIZE 480
#endif
#endif
