
#ifndef __ZARLINKCOMMONINIT_H__
#define __ZARLINKCOMMONINIT_H__

#include "zarlinkCommon.h"

BOOL zarlinkInitDevice( RTKDevObj *pDev );
int zarlinkCaculateDevObj(RTKDevType dev_type);
int zarlinkRegDevForEvHandle(RTKDevObj * pDev);
int rtkGetNewChID(void);
RTKLineObj * rtkGetLine(int chid);
RTKDevObj * rtkGetDev(int chid);

#endif /* __ZARLINKCOMMONINIT_H__ */


