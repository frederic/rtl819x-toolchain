#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <scsi/scsi.h>
/* #include <scsi/scsi_ioctl.h> */ /* glibc hides this file sometimes */

/* Test code for D. Gilbert's extensions to the Linux OS SCSI generic ("sg")
   device driver.
*  Copyright (C) 1999 D. Gilbert
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.

   This program does a SCSI inquiry command on the given device and 
   outputs some of the result. This program highlights the use of the
   SCSI_IOCTL_SEND_COMMAND ioctl. This should be able to be applied to 
   any SCSI device file descriptor (not just one related to sg). [Whether
   this is a good idea on a disk while it is mounted is debatable.
   No detrimental effects when this was tested ...]

Version 0.14 20011218
*/
        

typedef struct my_scsi_ioctl_command {
        unsigned int inlen;  /* _excluding_ scsi command length */
        unsigned int outlen;
        unsigned char data[1];  /* was 0 but that's not ISO C!! */
                /* on input, scsi command starts here then opt. data */
} My_Scsi_Ioctl_Command;

#define OFF (2 * sizeof(unsigned int))

#ifndef SCSI_IOCTL_SEND_COMMAND
#define SCSI_IOCTL_SEND_COMMAND 1
#endif

#define INQUIRY_CMD     0x12
#define INQUIRY_CMDLEN  6
#define INQUIRY_REPLY_LEN 96

#define RELEASE_CMD    0x17
#define RELEASE_CMDLEN  6

#define RESERVE_CMD    0x16
#define RESERVE_CMDLEN  6

int main(int argc, char * argv[])
{
    //int s_fd, res, k, to;
	int s_fd, res, k;
    unsigned char inqCmdBlk [INQUIRY_CMDLEN] = {INQUIRY_CMD, 0, 0, 0,
                                                INQUIRY_REPLY_LEN, 0};
    unsigned char relCmdBlk [RELEASE_CMDLEN] = {RELEASE_CMD, 0, 0, 0, 0, 0};
    unsigned char resCmdBlk [RESERVE_CMDLEN] = {RESERVE_CMD, 0, 0, 0, 0, 0};  
                                                                                          
    unsigned char* inqBuff = (unsigned char*)malloc(sizeof(My_Scsi_Ioctl_Command) + sizeof(inqCmdBlk) + 512);
    My_Scsi_Ioctl_Command * ishp = (My_Scsi_Ioctl_Command *)inqBuff;
    unsigned char* buffp = ishp->data;
    char * file_name = 0;
    int do_nonblock = 0;
    int oflags = 0;

    for (k = 1; k < argc; ++k) {
	if (0 == strcmp(argv[k], "-n"))
	    do_nonblock = 1;
	else if (*argv[k] != '-')
            file_name = argv[k];
	else {
	    printf("Unrecognized argument '%s'\n", argv[k]);
	    file_name = 0;
	    break;
	}
    }
    if (0 == file_name) {
        printf("Usage: 'smsp_eject [-n] <scsi_device>'\n");
        printf("     where: -n   open device in non-blocking mode\n");
        printf("  Examples: smsp_eject /dev/sda\n");
        printf("            smsp_eject /dev/sg0\n");
        printf("            smsp_eject -n /dev/scd0\n");
        return 1;
    }
    
    if (do_nonblock)
	oflags = O_NONBLOCK;
    s_fd = open(file_name, oflags | O_RDONLY);
    if (s_fd < 0) {
        perror("smsp_eject: open error");
        return 1;
    }
    /* Don't worry, being very careful not to write to a none-scsi file ... */
#if 0
    res = ioctl(s_fd, SCSI_IOCTL_GET_BUS_NUMBER, &to);
    if (res < 0) {
        /* perror("ioctl on scsi device, error"); */
        printf("smsp_eject: not a scsi device\n");
        return 1;
    }
#endif   
    /*send inquiry command*/
#if 0
    ishp->inlen = 0;
    ishp->outlen = INQUIRY_REPLY_LEN;
    memcpy(buffp, inqCmdBlk, INQUIRY_CMDLEN);
    res = ioctl(s_fd, SCSI_IOCTL_SEND_COMMAND, inqBuff);
    if (0 == res) {
        to = (int)*(buffp + 7);
        printf("    %.8s  %.16s  %.4s, byte_7=0x%x\n", buffp + 8,
               buffp + 16, buffp + 32, to);
    }
    else if (res < 0)
        perror("smsp_eject: SCSI_IOCTL_SEND_COMMAND err");
    else 
        printf("smsp_eject: SCSI_IOCTL_SEND_COMMAND status=0x%x\n", res);
#endif
    
    /*send release command*/    
    ishp->inlen = 0;
    ishp->outlen = 0;
    memcpy(buffp, relCmdBlk, RELEASE_CMDLEN);
    res = ioctl(s_fd, SCSI_IOCTL_SEND_COMMAND, inqBuff);
    if (0 == res) {
        printf("smsp_eject: SCSI_IOCTL_SEND_COMMAND ok\n");
    }
    else if (res < 0)
        perror("smsp_eject: SCSI_IOCTL_SEND_COMMAND err");
    else 
        printf("smsp_eject: SCSI_IOCTL_SEND_COMMAND status=0x%x\n", res);

    /*send reserve command*/
    ishp->inlen = 0;
    ishp->outlen = 0;
    memcpy(buffp, resCmdBlk, RESERVE_CMDLEN);
    res = ioctl(s_fd, SCSI_IOCTL_SEND_COMMAND, inqBuff);
    if (0 == res) {
        printf("smsp_eject: SCSI_IOCTL_SEND_COMMAND ok\n");
    }
    else if (res < 0)
        perror("smsp_eject: SCSI_IOCTL_SEND_COMMAND err");
    else 
        printf("smsp_eject: SCSI_IOCTL_SEND_COMMAND status=0x%x\n", res);     

    res = close(s_fd);
    if (res < 0) {
        perror("smsp_eject: close error");
        return 1;
    }
    return 0;
}
