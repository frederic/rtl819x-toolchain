/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: dallas_app_cmd.c,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.6 $
#-------------------------------------------------------------------------------
# Command for Dallas Controller application.
#-------------------------------------------------------------------------------
*/


#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
 
#include "dallas_app_cmd.h"
#include "dallas_app_util.h"
#include "zlib.h"

#define IMAGE_SIZE_DECOMPRESSED_MIN	3000000
#define IMAGE_SIZE_DECOMPRESSED_DEF	5500000

struct image_header {
  unsigned char magic_id[4];
  unsigned long program_size;
  unsigned char header_version;
  unsigned char compress_method;
  unsigned char image_type;
  unsigned char reserved_1;
  unsigned char name[32];
  unsigned long time; /* ANSI C time() format */
  unsigned long uncompress_size;
  unsigned long file_size;
  unsigned long checksum;
  unsigned char version[32];
  unsigned char vendor[32];
  unsigned char reserved2[132];
};


unsigned char DALLAS_ENABLE_GMII[44] = {
0x02, 0x00, 0x00, 0x80,     /* Opcode(set+ack) */
0x14, 0x30, 0x01, 0x60,     /* address=0x60013014*/
0x04, 0x00, 0x00, 0x00,     /* Size=0x00000004 */
0x00, 0x00, 0x00, 0x00,     /* pad */
0x00, 0x00, 0x04, 0x00,     /* value1=0x00040000, new gmii setting 20110531 change */
0x00, 0x00, 0x00, 0x00,     /* padding3 */
0x00, 0x00, 0x00, 0x00,     /* padding4 */
0x00, 0x00, 0x00, 0x00,     /* padding5 */
0x00, 0x00, 0x00, 0x00,     /* padding6 */
0x00, 0x00, 0x00, 0x00,     /* padding7 */
0x00, 0x00, 0x00, 0x00,     /* padding8 */
};

#define NUM_DDR_CONTROLLER_INIT_FRAMES 3
unsigned char DDR_CONTROLLER_INIT_FRAMES[NUM_DDR_CONTROLLER_INIT_FRAMES][256] = {
{   /* frame 0 */
0x02, 0x00, 0x00, 0x80,     /* opcode=0x80000002 (SET) */
0x00, 0x00, 0x00, 0xB0,     /* address=0xB0000000 */
0xD4, 0x00, 0x00, 0x00,     /* length=0x000000D4 */
0x00, 0x00, 0x00, 0x00,     /* pad=0 */
0x01, 0x01, 0x00, 0x00,     /* data=... */
0x00, 0x01, 0x01, 0x01,
0x01, 0x01, 0x00, 0x01,
0x01, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00,     /* 0x10 */
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x01,
0x00, 0x01, 0x00, 0x00,
0x00, 0x01, 0x00, 0x00,     /* 0x20 */
0x01, 0x01, 0x01, 0x00,
0x00, 0x00, 0x01, 0x02,
0x01, 0x03, 0x00, 0x04,
0x02, 0x00, 0x00, 0x00,     /* 0x30 */
0x03, 0x03, 0x02, 0x03,
0x02, 0x03, 0x02, 0x02,
0x02, 0x01, 0x03, 0x0F,
0x0A, 0x0a, 0x0a, 0x0F,     /* 0x40 */
0x02, 0x00, 0x02, 0x00,     
0x04, 0x02, 0x02, 0x04,
0x02, 0x02, 0x02, 0x03,
0x00, 0x00, 0x00, 0x00,     /* 0x50 */
0x05, 0x00, 0x02, 0x08,
0x00, 0x00, 0x00, 0x00,
0x00, 0x1e, 0x1e, 0x1e,
0x7F, 0x00, 0x00, 0x00,     /* 0x60 */
0x5F, 0x3d, 0x3d, 0x3d,
0x34, 0x00, 0xc3, 0x03,
0x00, 0x06, 0x02, 0x0e,
0xc2, 0x00, 0x00, 0x00,     /* 0x70 */
0x00, 0x00, 0x00, 0x00,
0xdd, 0x03, 0x00, 0x00,
0xFF, 0xFF, 0x00, 0x00,     /* changed by request of MosheI (23/12/07) */
0x00, 0x00, 0x00, 0x00,     /* 0x80 */
0x34, 0x00, 0xC8, 0x00,
0x02, 0x00, 0xe2, 0x22,
0x0f, 0x00, 0xC8, 0x00,
0x00, 0x00, 0x00, 0x00,     /* 0x90 */
0xc7, 0x63, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,     /* 0xA0 */
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,     /* 0xB0 */
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,     /* 0xC0 */
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,     /* 0xD0 */
},
{   /* frame 1 */
0x02, 0x00, 0x00, 0x80,     /* opcode=0x80000002 (SET) */
0x74, 0x00, 0x00, 0xB0,     /* address=0xB0000074 */
0x04, 0x00, 0x00, 0x00,     /* length=0x00000004 */
0x00, 0x00, 0x00, 0x00,    /* pad=0 */
0xFF, 0x0F, 0x00, 0x00,     /* data=0x00000FFF */
},
{   /* frame 2 */
0x02, 0x00, 0x00, 0x80,     /* opcode=0x80000002 (SET) */
0x20, 0x00, 0x00, 0xB0,     /* address=0xB0000020 */
0x04, 0x00, 0x00, 0x00,     /* length=0x00000004 */
0x00, 0x00, 0x00, 0x00,     /* pad=0 */
0x00, 0x01, 0x00, 0x01,     /* data=0x01000100 */
},
};

#define DDR_CONTROLLER_INIT_VERIFY_ADDR 0xB0000074
#define DDR_CONTROLLER_INIT_VERIFY_MASK 0x01000000

#define NUM_DDR_PHY_INIT_FRAMES 80
unsigned char DDR_PHY_INIT_FRAMES[NUM_DDR_PHY_INIT_FRAMES][24] = 
{
{   /* frame 0 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
},
{   /* frame 1 */ 
0x02, 0x00, 0x00, 0x80,
0x2c, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0xa0, 0x00, 0x00,
},
{   /* frame 2 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 3 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 4 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 5 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00,
},
{   /* frame 6 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x11, 0x02, 0x00, 0x00,
},
{   /* frame 7 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 8 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 9 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 10 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x05, 0x00, 0x00, 0x00,
},
{   /* frame 11 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x81, 0x61, 0x00, 0x00,
},
{   /* frame 12 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 13 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 14 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 15 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x06, 0x00, 0x00, 0x00,
},
{   /* frame 16 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x31, 0x1e, 0x00, 0x00,
},
{   /* frame 17 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 18 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 19 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 20 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x0b, 0x00, 0x00, 0x00,
},
{   /* frame 21 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x93, 0x24, 0x00, 0x00,
},
{   /* frame 22 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 23 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 24 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 25 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x0c, 0x00, 0x00, 0x00,
},
{   /* frame 26 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x02, 0x00, 0x00, 0x00,
},
{   /* frame 27 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 28 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 29 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 30 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x0d, 0x00, 0x00, 0x00,
},
{   /* frame 31 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x5c, 0xef, 0x00, 0x00,
},
{   /* frame 32 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 33 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 34 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 35 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x0e, 0x00, 0x00, 0x00,
},
{   /* frame 36 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x01, 0x13, 0x00, 0x00,
},
{   /* frame 37 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 38 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 39 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 40 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x28, 0x00, 0x00, 0x00,
},
{   /* frame 41 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x09, 0x36, 0x00, 0x00,
},
{   /* frame 42 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 43 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 44 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 45 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x29, 0x00, 0x00, 0x00,
},
{   /* frame 46 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x01, 0x30, 0x00, 0x00,
},
{   /* frame 47 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 48 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 49 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 50 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2a, 0x00, 0x00, 0x00,
},
{   /* frame 51 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x04, 0x36, 0x00, 0x00,
},
{   /* frame 52 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 53 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 54 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 55 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2b, 0x00, 0x00, 0x00,
},
{   /* frame 56 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x04, 0x04, 0x00, 0x00,
},
{   /* frame 57 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 58 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 59 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 60 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2c, 0x00, 0x00, 0x00,
},
{   /* frame 61 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x04, 0x04, 0x00, 0x00,
},
{   /* frame 62 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 63 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 64 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 65 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2d, 0x00, 0x00, 0x00,
},
{   /* frame 66 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
},
{   /* frame 67 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 68 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 69 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 70 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2e, 0x00, 0x00, 0x00,
},
{   /* frame 71 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x3a, 0x00, 0x00,
},
{   /* frame 72 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 73 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 74 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
{   /* frame 75 */ 
0x02, 0x00, 0x00, 0x80,
0x28, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x2f, 0x00, 0x00, 0x00,
},
{   /* frame 76 */ 
0x02, 0x00, 0x00, 0x80,
0x2C, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x3a, 0x00, 0x00,
},
{   /* frame 77 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x03, 0x00,
},
{   /* frame 78 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x01, 0x00,
},
{   /* frame 79 */ 
0x02, 0x00, 0x00, 0x80,
0x24, 0x00, 0x00, 0x20,
0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00,
},
};

extern unsigned char dallas_image_file[128];
extern unsigned char dallas_nvdb_file[128];

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_get
**  Description:
**  This function is used for getting value of specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**      The value of specified address will be set to buff field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_get(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_GET;
	ioctl(fd, PBRC_GET, pbrc);
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_set
**  Description:
**  This function is used for setting value to specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_set(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_SET;
	ioctl(fd, PBRC_SET, pbrc);
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_set_field
**  Description:
**  This function is used for setting value to specific address field on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
Notes:
**
*******************************************************************************/
static int pmc_pbrc_set_field(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_SET_FIELD;
	ioctl(fd, PBRC_SET_FIELD, pbrc);
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_branch
**  Description:
**  This function is used for jumping to specific address on Dallas.
**  Inputs:
**      pbrc 每 pointer to struct that contains command information.
**  Outputs:
**      When an error occurs, the corresponding error code will be inserted to error_code field of pbrc.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_branch(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_BRANCH;
	ioctl(fd, PBRC_BRANCH, pbrc);
	
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_read_lag
**  Description:
**  Read data (32 bits) from the chosen LAG ADDRESS.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_read_lag(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_READ_LAG;
	ioctl(fd, PBRC_READ_LAG, pbrc);
	
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_write_lag
**  Description:
**  Write DATA (32 bits) to the chosen LAG ADDRESS.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_write_lag(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_WRITE_LAG;
	ioctl(fd, PBRC_WRITE_LAG, pbrc);
	
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_read_uni_mac
**  Description:
**  Read data (32 bits) from the chosen ADDRESS of UNI MAC.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_read_uni_mac(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_READ_UNI_MAC;
	ioctl(fd, PBRC_READ_UNI_MAC, pbrc);
	
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_write_uni_mac
**  Description:
**  Write DATA (32 bits) to the chosen ADDRESS of UNI MAC.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_write_uni_mac(int fd, struct pbrc_cmd * pbrc)
{
	int retval = -1;
	
	pbrc->command = PBRC_WRITE_UNI_MAC;
	ioctl(fd, PBRC_WRITE_UNI_MAC, pbrc);
	
	if (pbrc->error_code == PBRC_OK) {
		retval = 0;
	}
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_reset
**  Description:
**  This function is used for resetting Dallas.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_reset(int fd)
{
	ioctl(fd, PBRC_RESET,0);

	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_init
**  Description:
**  This API is used for initializing Dallas.
**  Inputs:
**      mode - MODE_NON_SLAVE or MODE_SLAVE
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_init(int fd, int mode)
{
	int i, j;
	unsigned long * pul;
	struct pbrc_cmd cmd;
	if (mode == MODE_NON_SLAVE) {
		/* Enable Dallas GMII */
		pul = (unsigned long * )DALLAS_ENABLE_GMII;
		cmd.address = dallas_convert_endian(*(pul + 1));
		cmd.length = dallas_convert_endian(*(pul + 2));
		memset(cmd.buff, 0x0, sizeof(cmd.buff));
		memcpy(cmd.buff, (pul + 4), cmd.length);
		for ( j = 0; j < cmd.length/4; j++) {
			cmd.buff[j] = dallas_convert_endian(cmd.buff[j]);
		}
		pmc_pbrc_set(fd, &cmd);

		if (cmd.error_code != PBRC_OK) {
			printf("Failed to enable Dallas GMII mode.\n");
			return -1;
		}
		/* init DDR & phy */
		for (i = 0; i < NUM_DDR_PHY_INIT_FRAMES; i++) {
			pul = (unsigned long * )DDR_PHY_INIT_FRAMES[i];
			cmd.address = dallas_convert_endian(*(pul + 1));
			cmd.length = dallas_convert_endian(*(pul + 2));
			memset(cmd.buff, 0x0, sizeof(cmd.buff));
			memcpy(cmd.buff, (pul + 4), cmd.length);
			for ( j = 0; j < cmd.length/4; j++) {
				cmd.buff[j] = dallas_convert_endian(cmd.buff[j]);
			}
			pmc_pbrc_set(fd, &cmd);
			if (cmd.error_code != PBRC_OK) {
				printf("init DDR & phy failed.\n");
				return -1;
			}
		}
		/* init DDR controller */
		for (i = 0; i < NUM_DDR_CONTROLLER_INIT_FRAMES; i++) {
			pul = (unsigned long * )DDR_CONTROLLER_INIT_FRAMES[i];
			cmd.address = dallas_convert_endian(*(pul + 1));
			cmd.length = dallas_convert_endian(*(pul + 2));
			memset(cmd.buff, 0x0, sizeof(cmd.buff));
			memcpy(cmd.buff, (pul + 4), cmd.length);
			for ( j = 0; j < cmd.length/4; j++) {
				cmd.buff[j] = dallas_convert_endian(cmd.buff[j]);
			}
			pmc_pbrc_set(fd, &cmd);
			if (cmd.error_code != PBRC_OK) {
				printf("DDR controller init failed.\n");
				return -1;
			}
		}
		
		/* wait for DDR controller become ready */
		usleep(100);
		/* check the init_done bit is set */
		cmd.address = DDR_CONTROLLER_INIT_VERIFY_ADDR;
		cmd.length = 0x4;
		pmc_pbrc_get(fd, &cmd);
		if (cmd.error_code != PBRC_OK) {
			printf("DDR controller init failed. Set value failed.\n");
			return -1;
		}
		if (!(cmd.buff[0] & DDR_CONTROLLER_INIT_VERIFY_MASK)) {
			printf("DDR controller init failed, init_bit is not set. Register value=0x%08lx.\n", cmd.buff[0]);
			return -1;
		}
	} else if(mode == MODE_SLAVE) {
		/* Enable Dallas GMII */
		pul = (unsigned long * )DALLAS_ENABLE_GMII;
		cmd.address = dallas_convert_endian(*(pul + 1));
		cmd.length = dallas_convert_endian(*(pul + 2));
		memset(cmd.buff, 0x0, sizeof(cmd.buff));
		memcpy(cmd.buff, (pul + 4), cmd.length);
		for ( j = 0; j < cmd.length/4; j++) {
			cmd.buff[j] = dallas_convert_endian(cmd.buff[j]);
		}
		pmc_pbrc_set(fd, &cmd);

		if (cmd.error_code != PBRC_OK) {
			printf("Failed to enable Dallas GMII mode.\n");
			return -1;
		}

		/* Enable Dallas' interrupt */		
		cmd.address = DALLAS_M_IRQ_MASK;
		cmd.length = 4;
		memset(cmd.buff, 0x0, sizeof(cmd.buff));
		cmd.buff[0] = DALLAS_M_IRQ_MASK_VALUE;
		pmc_pbrc_set(fd, &cmd);
		if (cmd.error_code != PBRC_OK) {
			printf("Failed to unmask Dallas GPIO IRQ.\n");
			return -1;
		}
		
		cmd.address = DALLAS_M_GPIO_RGF_IRQ_MASK;
		cmd.length = 4;
		memset(cmd.buff, 0x0, sizeof(cmd.buff));
		cmd.buff[0] = DALLAS_M_GPIO_RGF_IRQ_MASK_VALUE;
		pmc_pbrc_set(fd, &cmd);
		if (cmd.error_code != PBRC_OK) {
			printf("Failed to unmask Dallas GPIO in.\n");
			return -1;
		}
	}
	
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_load_fw
**  Description:
**  This API is used for loading firmware to Dallas.
**  Inputs:
**      mode - MODE_NON_SLAVE or MODE_SLAVE
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
static int pmc_pbrc_load_fw(int fd, int mode)
{
	unsigned long count = 0;
	unsigned long image_size;
	unsigned long addr = 0;
	FILE * image_fd = NULL, *nvdb_fd=NULL;
	unsigned char buff[1024];
	struct pbrc_cmd cmd;
	int ret = 0;
	int read_bytes = 0;
	unsigned char * buff_src = NULL;
	unsigned char * buff_dest = NULL;
	int i;
	struct stat buf;
	unsigned long file_size;
	struct image_header *header;
	struct sysinfo info;
	unsigned long total_ram = 0;
	unsigned long total_swap = 0;
	
	dallas_nvdb s_nvdb={0x30305253,
											0x40,
											0x62d50c00,
											0x00000390,
											0xac123d1e,
											0xffffff00,
											0x62d50c00,
											0x00000490, 
											0xac123d1f,
											0xffffff00,
											0x62d50c00,
											0x00000590,
											0xac123d20,
											0xffffff00, 
											0x01,
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01, 
											0x01  };
	int return_no;

	if (ret = sysinfo(&info) != 0){
		printf("Failed to get system info.\n");
		return -1;
	}

	total_ram = info.totalram;
	total_swap = info.totalswap;

	if (mode == MODE_NON_SLAVE) {
		/*load NVDB, if there is one*/
		if(stat(dallas_nvdb_file, &buf)<0) {
			printf("No nvdb file specified, booting with default configurations.\n");
		} else {
			if ( NULL == (nvdb_fd = fopen(dallas_nvdb_file, "rt" ))) {
				printf("File %s open failed, booting with default configurations.\n", dallas_nvdb_file);
			} else {
				return_no=dallas_get_nvdb_file(&s_nvdb, nvdb_fd);
				
				if(return_no!=0) {
					/*Do not load the error parameters into Dallas, will cause unexpected result.*/
					printf("Error occurs when getting the parameters from NVDB file, booting with default configurations.\n");
				} else {
					printf("Loading NVDB:");
					
					
					cmd.address = DALLAS_NVDB_ADDRESS;
					cmd.length = sizeof(dallas_nvdb);
					memcpy(cmd.buff, (unsigned char *)&s_nvdb, cmd.length);
		
					/*for ( i = 1; i < read_bytes/4; i++) {
						cmd.buff[i] = dallas_convert_endian(cmd.buff[i]);
					}*/
					
					for(i=0; i<(cmd.length)/4; i++) {
						printf("0x%x,\n", cmd.buff[i]);		
					}
		
					/* send data to dallas */
					pmc_pbrc_set(fd, &cmd);
					
					switch (cmd.error_code) {
					case PBRC_E_INVALID_CMD:
						printf("Error: Unknown opcode value\n");
						break;
					case PBRC_E_ADDRESS:
						printf("Error: Address unaligned or out of range\n");
						break;
					case PBRC_E_LENGTH:
						printf("Error: Length unaligned or larger than max size\n");
						break;
					case PBRC_E_SOCKET:
						printf("Error: snd&rcv error\n");
						break;
						case PBRC_E_TIMEOUT:
						printf("Error: Receive timeout\n");
						break;
					case PBRC_OK:
						printf(" Done!\n");
						break;
					default:
						printf("Error: Unknown error!\n");
						break;
					}
				}
				fclose(nvdb_fd);
			}
		}
		
		/*load firmware image*/
		if(stat(dallas_image_file, &buf)<0) {
			return -1;
		}
		
		file_size = (unsigned long)buf.st_size;

		/* Decompress Dallas firmware. */
		buff_src = (unsigned char *)malloc(file_size + (4 - file_size%4));
		if (buff_src == NULL) {
			printf("Memory allocated failed.\n");
			return -1;
		}
		
		memset(buff_src, 0, file_size + (4 - file_size%4));
		
		/* load image to source buffer */
		if ( NULL == (image_fd = fopen(dallas_image_file, "rb" ))) {
			printf("File %s open failed \n", dallas_image_file);
			free(buff_src);
			return -1;
		}
		
		while ((read_bytes = fread(buff, sizeof(unsigned char), 1024, image_fd)) != 0) {
			memcpy(buff_src + count, buff, read_bytes);
			count += read_bytes;
		}

		/* Check the image header */
		header = (struct image_header *)buff_src;
		/* compress_method is 1 for ZLIB, image_type is 1 for 7140 */
		if((header->compress_method != 1) || (header->image_type != 1) || (header->file_size != file_size)) {
			printf("Unsupported image type.\n");
			free(buff_src);
			return -1;
		}
		
		image_size = file_size * 4;
		if ((image_size +  file_size + (4 - file_size%4)) > (total_ram + total_swap - 0x100000)) {
			image_size = IMAGE_SIZE_DECOMPRESSED_DEF;
		} else if (image_size < IMAGE_SIZE_DECOMPRESSED_MIN) {
			image_size = IMAGE_SIZE_DECOMPRESSED_MIN;
		}

		buff_dest = (unsigned char *)malloc(image_size);
		if (buff_dest == NULL) {
			printf("Memory allocated failed.\n");
			free(buff_src);
			return -1;
		}
		memset(buff_dest, 0, image_size);

		ret = uncompress(buff_dest, &image_size, buff_src + COMPRESSED_IMAGE_HEADER_SIZE, count - COMPRESSED_IMAGE_HEADER_SIZE);	
		if (0 != ret) {
			printf("Uncompress failed. Error=%d.\n", ret);
			free(buff_src);
			free(buff_dest);
			fclose(image_fd);
			return -1;
		}
	
		/* Load image to Dallas */
		printf("Loading image:\n");

		addr = DALLAS_START_ADDRESS;
		count = 0;
		read_bytes = 0;
		while (1) {
			if ((image_size - count) > 1024) {
				read_bytes = 1024;
			} else if ((image_size - count) > 0) {
				read_bytes = image_size - count;
			} else {
				break;
			}
			
			count += read_bytes;
			/* the length must be aligned */
			if (read_bytes%4) {
				read_bytes += 4 - read_bytes%4;
			}
			
			cmd.address = addr;
			cmd.length = read_bytes;
			memcpy(cmd.buff, buff_dest + count - read_bytes, read_bytes);

			for ( i = 0; i < read_bytes/4; i++) {
				cmd.buff[i] = dallas_convert_endian(cmd.buff[i]);
			}

			addr = addr + read_bytes;
			/* send data to dallas */
			pmc_pbrc_set(fd, &cmd);
			
			switch (cmd.error_code) {
			case PBRC_E_INVALID_CMD:
				printf("Error: Unknown opcode value\n");
				break;
			case PBRC_E_ADDRESS:
				printf("Error: Address unaligned or out of range\n");
				break;
			case PBRC_E_LENGTH:
				printf("Error: Length unaligned or larger than max size\n");
				break;
			case PBRC_E_SOCKET:
				printf("Error: snd&rcv error\n");
				break;
				case PBRC_E_TIMEOUT:
				printf("Error: Receive timeout\n");
				break;
			case PBRC_OK:
				break;
			default:
				printf("Error: Unknown error!\n");
				break;
			}
			if (cmd.error_code) {
				printf("\nLoad firmware failed!\n");
				ret = -1;
				break;
			}
			if (!(count%102400)) {
				printf(".");
				fflush(stdout);
			}
		}
		printf("\nTotal sent=%ld Bytes.\n", count);
		free(buff_src);
		free(buff_dest);
		fclose(image_fd);
	} else if (mode == MODE_NON_SLAVE_FPGA) {
		if ( NULL == (image_fd = fopen(dallas_image_file, "rb" ))) {
			printf("File %s open failed \n", dallas_image_file);
			return -1;
		}
		
		printf("Loading image:\n");
		addr = DALLAS_START_ADDRESS_FPGA;
		while ((ret = fread(buff, sizeof(unsigned char), 1024, image_fd)) != 0) {
			count += ret;
			/* the length must be aligned */
			if (ret%4)
			{
				ret += 4 - ret%4;
			}
			
			memcpy(cmd.buff, buff, ret);
			for ( i = 0; i < ret/4; i++) {
				cmd.buff[i] = dallas_convert_endian(cmd.buff[i]);
			}
			cmd.address = addr;
			cmd.length = ret;
			addr = addr + ret;
			/* send data to dallas */
			pmc_pbrc_set(fd, &cmd);
			
			switch (cmd.error_code) {
			case PBRC_E_INVALID_CMD:
				printf("Error: Unknown opcode value\n");
				break;
			case PBRC_E_ADDRESS:
				printf("Error: Address unaligned or out of range\n");
				break;
			case PBRC_E_LENGTH:
				printf("Error: Length unaligned or larger than max size\n");
				break;
			case PBRC_E_SOCKET:
				printf("Error: snd&rcv error\n");
				break;
			case PBRC_E_TIMEOUT:
				printf("Error: Receive timeout\n");
				break;
			case PBRC_OK:
				break;
			default:
				printf("Error: Unknown error!\n");
				break;
			}
			if (cmd.error_code) {
				printf("\nLoad firmware failed!\n");
				ret = -1;
				break;
			}
			if (!(count%102400)) {
				printf(".");
				fflush(stdout);
			}
		}
		printf("\nTotal sent=%ld Bytes.\n", count);
		fclose(image_fd);
	} else if (mode == MODE_SLAVE) {
		if ( NULL == (image_fd = fopen(dallas_image_file, "rb" ))) {
			printf("File %s open failed \n", dallas_image_file);
			return -1;
		}
		
		addr = DALLAS_ISR_CODE_START_ADDRESS;
		while ((ret = fread(buff, sizeof(unsigned char), 1024, image_fd)) != 0) {
			count += ret;
			/* the length must be aligned */
			if (ret%4)
			{
				ret += 4 - ret%4;
			}
			
			memcpy(cmd.buff, buff, ret);
			for ( i = 0; i < ret/4; i++) {
				cmd.buff[i] = dallas_convert_endian(cmd.buff[i]);
			}
			cmd.address = addr;
			cmd.length = ret;
			addr = addr + ret;
			/* send data to dallas */
			pmc_pbrc_set(fd, &cmd);
			
			switch (cmd.error_code) {
			case PBRC_E_INVALID_CMD:
				printf("Error: Unknown opcode value\n");
				break;
			case PBRC_E_ADDRESS:
				printf("Error: Address unaligned or out of range\n");
				break;
			case PBRC_E_LENGTH:
				printf("Error: Length unaligned or larger than max size\n");
				break;
			case PBRC_E_SOCKET:
				printf("Error: snd&rcv error\n");
				break;
			case PBRC_E_TIMEOUT:
				printf("Error: Receive timeout\n");
				break;
			case PBRC_OK:
				break;
			default:
				printf("Error: Unknown error!\n");
				break;
			}
			if (cmd.error_code) {
				printf("\nLoad firmware failed!\n");
				ret = -1;
				break;
			}
		}
		fclose(image_fd);
	} else {
		printf("Error: unknown mode.\n");
	}
	
	return ret;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_start
**  Description:
**  This function is used for kicking off Dallas.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
Notes:
**
*******************************************************************************/
static int pmc_pbrc_start(int fd, int mode)
{
	/* Invoke pmc_pbrc_branch to jump to start address on Dallas firmware. */
	struct pbrc_cmd cmd;
	int retval = -1;
	
	cmd.command = PBRC_BRANCH;
	if (mode == MODE_NON_SLAVE_FPGA) {
		cmd.address = DALLAS_START_ADDRESS_FPGA;
	} else if (mode == MODE_NON_SLAVE){
		cmd.address = DALLAS_START_ADDRESS;
	} else if (mode == MODE_SLAVE) {
		cmd.address = DALLAS_ENABLE_INTERRUPT_ADDRESS;
	} else {
		printf("Error: unknown mode!\n");
	}
	retval = pmc_pbrc_branch(fd, &cmd);
	
	return retval;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_non_slave
**  Description:
**  This function is used for running Dallas in non-slave mode. 
**  Inputs:
**  
**  Outputs:
**      
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_non_slave(int fd, int mode)
{
	int rev = -1;

	pmc_pbrc_reset(fd);
	if (mode == MODE_NON_SLAVE) {
		rev = pmc_pbrc_init(fd, mode);
		if (0 != rev) {
			return rev;
		}
	}

	rev = pmc_pbrc_load_fw(fd, mode);
	if (0 != rev) {
		return -1;
	}

	rev = pmc_pbrc_start(fd, mode);
	if (0 != rev) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_slave
**  Description:
**  This function is used for running Dallas in slave mode. 
**  Inputs:
**  
**  Outputs:
**      
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**  This function only initialize the device, GPON related work would be done by GPON Driver & Stack and GPON Application.
**
*******************************************************************************/
int pmc_pbrc_slave(int fd)
{
	int rev;
	struct pbrc_cmd cmd;

	/* Set GPIO[16] to low */
	pmc_pbrc_dallas_irq(fd);
	/* reset Dallas */
	pmc_pbrc_reset(fd);
	rev = pmc_pbrc_init(fd, MODE_SLAVE);
	if (0 != rev) {
		return rev;
	}

	rev = pmc_pbrc_load_fw(fd, MODE_SLAVE);
	if (0 != rev) {
		return -1;
	}

	rev = pmc_pbrc_start(fd, MODE_SLAVE);
	if (0 != rev) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_reset_debug
**  Description:
**  This function is used for resetting Dallas. For debug.
**  Inputs:
**  
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_reset_debug(int fd)
{
	/* Invoke pmc_pbrc_reset to reset Dallas. */
	pmc_pbrc_reset(fd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_get_debug
**  Description:
**  This function is used for getting value of specific address on Dallas. For debug.
**  Inputs:
**      address 每 target address.
**      length 每 address length
**  Outputs:
**      Value of specific address on Dallas.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_get_debug(int fd, unsigned long address, unsigned long length)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_GET;
	cmd.address = address;
	cmd.length = length;
	dallas_show_request(&cmd);
	pmc_pbrc_get(fd, &cmd);
	/* Show returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_set_debug
**  Description:
**  This function is used for setting value to specific address on Dallas. For debug.
**  Inputs:
**      address 每 target address.
**      length 每 address length
**      value 每 value should be set to target address
**  Outputs:
**      Command returned value.
**  Return:
**      0 每 success
**      non-zero - fail
Notes:
**
*******************************************************************************/
int pmc_pbrc_set_debug(int fd, unsigned long address, unsigned long length, unsigned long * value)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_SET;
	cmd.address = address;
	cmd.length = length;
	memcpy(cmd.buff, value, PBRC_DATA_MAX_LENGTH*sizeof(unsigned long));
	dallas_show_request(&cmd);
	pmc_pbrc_set(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_set_field_debug
**  Description:
**  This function is used for setting value to specific address field on Dallas. For debug.
**  Inputs:
**      address 每 target address.
**      mask 每 mask of this operation
**      value 每 value should be set to target address field
**  Outputs:
**      Command returned value.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_set_field_debug(int fd, unsigned long address, unsigned long mask, unsigned long * value)
{
	/* Invoke pmc_pbrc_set_field to set value to specific address on Dallas. */
	/* Show command returned value.  */
	
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_SET_FIELD;
	cmd.address = address;
	cmd.mask = mask;
	memcpy(cmd.buff, value, PBRC_DATA_MAX_LENGTH*sizeof(unsigned long));
	dallas_show_request(&cmd);
	pmc_pbrc_set_field(fd, &cmd);
	dallas_show_reply(&cmd);
	
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_branch_debug
**  Description:
**  This function is used for jumping to specific address on Dallas. For debug.
**  Inputs:
**      address 每 target address
**  Outputs:
**      Command returned value.
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_branch_debug(int fd, unsigned long address)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_BRANCH;
	cmd.address = address;
	dallas_show_request(&cmd);
	pmc_pbrc_branch(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_read_uni_mac
**  Description:
**  Read data (32 bits) from the chosen ADDRESS of UNI MAC.
**  Inputs:
**      fd - File descriptor for the operation. 
**      address 每 target address
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_read_uni_mac_debug(int fd, unsigned long address)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_READ_UNI_MAC;
	cmd.address = address;
	dallas_show_request(&cmd);
	pmc_pbrc_read_uni_mac(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_write_uni_mac
**  Description:
**  Write DATA (32 bits) to the chosen ADDRESS of UNI MAC.
**  Inputs:
**      fd - File descriptor for the operation. 
**      address 每 target address
**      value -  data be written to specific address
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_write_uni_mac_debug(int fd, unsigned long address, unsigned long * value)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_WRITE_UNI_MAC;
	cmd.address = address;
	memcpy(cmd.buff, value, PBRC_DATA_MAX_LENGTH*sizeof(unsigned long));
	dallas_show_request(&cmd);
	pmc_pbrc_write_uni_mac(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_read_lag
**  Description:
**  Read data (32 bits) from the chosen LAG ADDRESS.
**  Inputs:
**      fd - File descriptor for the operation. 
**      address 每 target address
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_read_lag_debug(int fd, unsigned long address)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_READ_LAG;
	cmd.address = address;
	dallas_show_request(&cmd);
	pmc_pbrc_read_lag(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_write_lag
**  Description:
**  Write DATA (32 bits) to the chosen LAG ADDRESS.
**  Inputs:
**      fd - File descriptor for the operation. 
**      address 每 target address
**      value -  data be written to specific address
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_write_lag_debug(int fd, unsigned long address, unsigned long * value)
{
	struct pbrc_cmd cmd;
	
	cmd.command = PBRC_WRITE_LAG;
	cmd.address = address;
	memcpy(cmd.buff, value, PBRC_DATA_MAX_LENGTH*sizeof(unsigned long));
	dallas_show_request(&cmd);
	pmc_pbrc_write_lag(fd, &cmd);
	/* Show command returned value. */
	dallas_show_reply(&cmd);
	return 0;
}

/*******************************************************************************
**
**  FUNCTION: pmc_pbrc_init
**  Description:
**  This function is used for initializing Dallas. For debug.
**  Inputs:
**      fd - File descriptor for the operation. 
**  Outputs:
**  
**  Return:
**      0 每 success
**      non-zero - fail
**  Notes:
**
*******************************************************************************/
int pmc_pbrc_init_debug(int fd, int mode)
{
	return pmc_pbrc_init(fd, mode);
}

int pmc_pbrc_dallas_irq(fd)
{
	ioctl(fd, DALLAS_IRQ, 0);
	return 0;
}

