/*
** $Id: sys_driv_type.h,v 1.1 2008-01-15 06:33:19 kennylin Exp $
** 
** This file is system specific and should be edited for your hardware platform
**
** This file is used by proslic_timer_intf.h and proslic_spiGci.h
*/
#ifndef DRIV_TYPE_H
#define DRIV_TYPE_H
#include "si_voice_datatypes.h"

#include "spi/spi.h"

/*
** SPI/GCI structure
*/
typedef struct{
	//uInt16 portID;	// chmap don't use this 
	rtl_spi_dev_t spi_dev;
} ctrl_S;

/*
** System timer interface structure 
*/
typedef struct{
	//_int64 ticksPerSecond;
	unsigned long ticksPerSecond;
} systemTimer_S;

/*
** System time stamp
*/
typedef struct{
	//_int64 time;
	unsigned long time;
} timeStamp;

#endif
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2007/02/15 23:33:25  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
*/
