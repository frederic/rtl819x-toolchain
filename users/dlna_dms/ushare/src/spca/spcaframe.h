#ifndef SPCAFRAME_H
#define SPCAFRAME_H
#include <sys/time.h>
#include <stdio.h>

struct frame_t{
	char header[5];
	int nbframe;
	double seqtimes;
	int deltatimes;
	int w;
	int h;
	int size;
	int format;
	unsigned short bright;
	unsigned short contrast;
	unsigned short colors;
	unsigned short exposure;
	unsigned char wakeup;
	int acknowledge;
	char movflag;
	char picflag;
	} __attribute__ ((packed));  

struct client_t{
	char message[4];
	unsigned char x;
	unsigned char y;
	unsigned char fps;
	unsigned char updobright;
	unsigned char updocontrast;
	unsigned char updocolors;
	unsigned char updoexposure;
	unsigned char updosize;
	unsigned char sleepon;
	} __attribute__ ((packed));
	

#endif
