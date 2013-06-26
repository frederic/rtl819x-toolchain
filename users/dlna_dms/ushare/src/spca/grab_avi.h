
#ifndef GRAB_AVI_H
#define GRAB_AVI_H
#include "avilib.h"
#include "spcav4l.h"
#include "spcaframe.h"
#include "tcputils.h"

typedef struct _ack
{
	char MovFlag;
	char PicFlag;
	
} Ack;
void dump_frame( struct frame_t* f );
int init_dev( char* videodevice, int grabmethod);

int newfile(char* fileName);
void newServer(void *port);
void loopFile();
void loopSock();
void grab (void);
void grab_ctrl(struct ushare_t *ut);


#endif
