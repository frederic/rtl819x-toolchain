#ifndef JCONFIG_H
#define JCONFIG_H

#define IMAGE_WIDTH 320 
#define IMAGE_HEIGHT 240

#define VIDEO_PALETTE_RAW_JPEG  20
#define VIDEO_PALETTE_JPEG  21
#define SWAP_RGB 1
int bpp;
/* our own ioctl */
struct video_param {
	int chg_para;
#define CHGABRIGHT 1
#define CHGQUALITY 2
#define CHGLIGHTFREQ 3
#define CHGTINTER  4
	__u8 autobright;
	__u8 quality;
	__u16 time_interval;
	__u8 light_freq;
};

/* Our private ioctl */
#define SPCAGVIDIOPARAM _IOR('v',BASE_VIDIOCPRIVATE + 1,struct video_param)
#define SPCASVIDIOPARAM _IOW('v',BASE_VIDIOCPRIVATE + 2,struct video_param)

/* Camera type jpeg yuvy yyuv yuyv grey gbrg*/
enum {  
	JPEG = 0,
	YUVY,
	YYUV,
	YUYV,
	GREY,
	GBRG,
};

#define BRIDGE_SPCA505 0
#define BRIDGE_SPCA506 1
#define BRIDGE_SPCA501 2
#define BRIDGE_SPCA508 3
#define BRIDGE_SPCA504 4
#define BRIDGE_SPCA500 5
#define BRIDGE_SPCA504B 6
#define BRIDGE_SPCA533 7
#define BRIDGE_SPCA504C 8
#define BRIDGE_SPCA561 9
#define BRIDGE_SPCA536 10
#define BRIDGE_SONIX 11
#define BRIDGE_ZR364XX 12
#define BRIDGE_ZC3XX 13
#define BRIDGE_CX11646 14
#define BRIDGE_TV8532 15
#define BRIDGE_ETOMS 16
#define BRIDGE_SN9CXXX 17
#define BRIDGE_MR97311 18
#define MAX_BRIDGE 19

struct palette_list {
	int num;
	const char *name;
};

struct bridge_list {
	int num;
	const char *name;
};

static struct bridge_list Blist[]={

	{BRIDGE_SPCA505,"SPCA505"},
	{BRIDGE_SPCA506,"SPCA506"},
	{BRIDGE_SPCA501,"SPCA501"},
	{BRIDGE_SPCA508,"SPCA508"},
	{BRIDGE_SPCA504,"SPCA504"},
	{BRIDGE_SPCA500,"SPCA500"},
	{BRIDGE_SPCA504B,"SPCA504B"},
	{BRIDGE_SPCA533,"SPCA533"},
	{BRIDGE_SPCA504C,"SPCA504C"},
	{BRIDGE_SPCA561,"SPCA561"},
	{BRIDGE_SPCA536,"SPCA536"},
	{BRIDGE_SONIX,"SN9C102"},
	{BRIDGE_ZR364XX,"ZR364XX"},
	{BRIDGE_ZC3XX,"ZC301-2"},
	{BRIDGE_CX11646,"CX11646"},
	{BRIDGE_TV8532, "TV8532"},
  	{BRIDGE_ETOMS,"ET61XX51"},
	{BRIDGE_SN9CXXX,"SN9CXXX"},
	{BRIDGE_MR97311,"MR97311"},
	{-1,NULL}
};
/* Camera type jpeg yuvy yyuv yuyv grey gbrg*/
static struct palette_list Plist[] ={
	{JPEG,"JPEG"},
	{YUVY,"YUVY"},
	{YYUV,"YYUV"},
	{YUYV,"YUYV"},
	{GREY,"GREY"},
	{GBRG,"GBRG"},
	{-1,NULL}
};

#define GET_EXT_MODES(bridge) (\
	(bridge) == BRIDGE_SPCA500 ? spca500_ext_modes : \
	(bridge) == BRIDGE_SPCA501 ? spca501_ext_modes : \
	(bridge) == BRIDGE_SPCA504 ? spca504_ext_modes : \
	(bridge) == BRIDGE_SPCA504B ? spca504_ext_modes : \
	(bridge) == BRIDGE_SPCA504C ? spca504_pccam600_ext_modes : \
	(bridge) == BRIDGE_SPCA506 ? spca506_ext_modes : \
	(bridge) == BRIDGE_SPCA508 ? spca508_ext_modes : \
	(bridge) == BRIDGE_SPCA533 ? spca533_ext_modes : \
	(bridge) == BRIDGE_SPCA561 ? spca561_ext_modes : \
	(bridge) == BRIDGE_SPCA536 ? spca536_ext_modes : \
	(bridge) == BRIDGE_SONIX ? sonix_ext_modes : \
	(bridge) == BRIDGE_ZR364XX ? zr364xx_ext_modes : \
	(bridge) == BRIDGE_ZC3XX ? zc3xx_ext_modes : \
	(bridge) == BRIDGE_CX11646 ? cx11646_ext_modes : \
	(bridge) == BRIDGE_SN9CXXX ? sn9c102p_ext_modes : \
	spca50x_ext_modes)

static unsigned int sn9c102p_ext_modes[][3] = {
	/* x , y , Code */
	{640, 480, 0x00},
	{352, 288, 0x10},	
	{320, 240, 0x01},
	{176, 144, 0x11},
	{0, 0, 0}
};
static unsigned int cx11646_ext_modes[][3] = {
	/* x , y , Code */
	{640, 480, 0x00},
	{352, 288, 0x01},	
	{320, 240, 0x02},
	{176, 144, 0x03},
	{0, 0, 0}
};
static unsigned int zc3xx_ext_modes[][3] = {
	/* x , y , Code */
	{640, 480, 0x00},
	{352, 288, 0x10},	
	{320, 240, 0x01},
	{176, 144, 0x11},
	{0, 0, 0}
};

static unsigned int zr364xx_ext_modes[][3] = {
	/* x , y , Code */
	{640, 480, 0x00},	
	{320, 240, 0x01},
	{160, 120, 0x02},
	{0, 0, 0}
};

static unsigned int sonix_ext_modes[][3] = {
	/* x , y , Code */
	{640, 480, 0x00},	
	{320, 240, 0x01},
	{160, 120, 0x02},
	{0, 0, 0}
};

static unsigned int spca500_ext_modes[][3] = {
	/* x , y , Standard*/
	{640, 480, 0x00},
	{352, 288, 0x00},
	{320, 240, 0x00},
	{176, 144, 0x00},
	{0, 0, 0}
};

static unsigned int spca501_ext_modes[][3] = {
	
	{640, 480, 0x00},
	{352, 288, 0x00},
	{320, 240, 0x00},
	{176, 144, 0x00},
	{160, 120, 0x00},
	{0, 0, 0}
};

static unsigned int spca504_ext_modes[][3] =
{	
	{ 640, 480, 0x01},
	{ 384, 288, 0x11},
	{ 352, 288, 0x11},
	{ 320, 240, 0x02},
	{ 192, 144, 0x12},
	{ 176, 144, 0x12},
	{ 0, 0, 0}
};

 
static unsigned int spca504_pccam600_ext_modes[][3] =
{	
	{ 1024, 768, 0x00}, 
	{ 640,  480, 0x01},
	{ 352,  288, 0x02},
	{ 320,  240, 0x03},
	{ 176,  144, 0x04},
	{ 0, 0, 0}
};


static unsigned int spca506_ext_modes[][3] =
{	
	{ 640, 480, 0x00},
	{ 352, 288, 0x01},
	{ 320, 240, 0x02},
	{ 176, 144, 0x04}, 
	{ 160, 120, 0x05},
	{ 0, 0, 0}
};

static unsigned int spca508_ext_modes[][3] =
{	
	{ 352, 288, 0x00},
	{ 320, 240, 0x01},
	{ 176, 144, 0x02},
	{ 160, 120, 0x03},
	{ 0, 0, 0}
};

static unsigned int spca533_ext_modes[][3] =
{	
	{ 640, 480, 0x41},
	{ 464, 480, 0x01},//PocketDVII unscaled resolution aspect ratio need to expand x axis
	{ 464, 352, 0x01},//Gsmart LCD3 feature good aspect ratio
	{ 384, 288, 0x11},
	{ 352, 288, 0x11},
	{ 320, 240, 0x02},
	{ 192, 144, 0x12},
	{ 176, 144, 0x12},
	{ 0, 0, 0}
};

static unsigned int spca561_ext_modes[][3] =
{
	{ 352, 288, 0x00},
	{ 320, 240, 0x01},
	{ 176, 144, 0x02},
	{ 160, 120, 0x03},
	{ 0, 0, 0}
};

static unsigned int spca536_ext_modes[][3] =
{
	/* x , y , Code */
	{ 464, 480, 0x01},
	{ 464, 352, 0x01},
	{ 384, 288, 0x11},
	{ 352, 288, 0x11},
	{ 320, 240, 0x02},
	{ 192, 144, 0x12},
	{ 176, 144, 0x12},	
	{ 0, 0, 0}
};

static unsigned int spca50x_ext_modes[][3] =
{	
	{ 640, 480, 0x00},
	{ 352, 288, 0x01},
	{ 320, 240, 0x02}, 
	{ 176, 144, 0x04}, 
	{ 160, 120, 0x05}, 
	{ 0, 0, 0}
};


#endif /* JCONFIG_H */
