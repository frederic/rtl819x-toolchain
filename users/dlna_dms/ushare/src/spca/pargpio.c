/***************************************************************************#
# pargpio: library to use parpaort device with 2 D/A . converter            #
# can read status pin 13 or pin 15                                          #
#.                                                                          #
# 		Copyright (C) 2005 Michel Xhaard                            #
#                                                                           #
# This program is free software; you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation; either version 2 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
# GNU General Public License for more details.                              #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program; if not, write to the Free Software               #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA #
#                                                                           #
#***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/parport.h>
#include <linux/ppdev.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

/* paraport 2 channels with D/A converter read signal pin 15 or pin 13 */
int openclaimParaport(char *dev)
{
	int fd,i;

	fd = open(dev, O_RDWR );
        if (fd < 0) {
                printf("ERROR: cannot open device %s\n",dev);
		return -1;
        }
	if(ioctl(fd, PPCLAIM) < 0){
	printf("ERROR: could not claim parport. Did you load the modules parport_pc, ppdev and parport? Check with /sbin/lsmod\n");
	close(fd);
	return -1;
	}
	return(fd);

}

unsigned char read_portstatus15(int fd)
{	
	unsigned char stat =0;
	unsigned char on = 0;
	int rc;
	rc=ioctl(fd, PPRSTATUS, &stat);
	on = (stat & 0x08)?1:0 ;
	//printf ("status 0x%02X \n",on);
	return on;
}
int closereleaseParaport( int fd)
{
	if(ioctl(fd,PPRELEASE) < 0){
	printf("erreur release PartPort \n");
	return -1;
	}
	close(fd);
	return 0;
}
unsigned char read_portstatus13(int fd)
{	
	unsigned char stat =0;
	unsigned char on = 0;
	int rc;
	rc=ioctl(fd, PPRSTATUS, &stat);
	on = (stat & 0x40)?1:0 ;
	//printf ("status 0x%02X \n",on);
	return on;
}
int port_setdata2(int fd,unsigned char val2)
{
	int rc;
	struct ppdev_frob_struct frob;
	
	frob.mask = PARPORT_CONTROL_STROBE ;
	/* set the strobe line*/		
	frob.val = PARPORT_CONTROL_STROBE ;// 1;toggle
	ioctl(fd,PPFCONTROL,&frob);
	rc=ioctl(fd, PPWDATA, &val2);
	frob.val = 0;
	ioctl(fd,PPFCONTROL,&frob);
	return(rc);
}
int port_setdata1(int fd,unsigned char bitval)
{
	int rc;
        struct ppdev_frob_struct frob;
	
        frob.mask = PARPORT_CONTROL_SELECT;
        frob.val= 0;//~PARPORT_CONTROL_SELECT;//0;	
	ioctl(fd,PPFCONTROL,&frob);
	rc = port_setdata2(fd,bitval);
        frob.val= PARPORT_CONTROL_SELECT; //8;		
	ioctl(fd,PPFCONTROL,&frob);	
return(rc);
}
int port_toggleInitbit (int fd)
{
        struct ppdev_frob_struct frob;
	static char value = 0;
	value = (value)? 0: PARPORT_CONTROL_INIT;
        frob.mask = PARPORT_CONTROL_INIT;
        frob.val= value;//0;	
	ioctl(fd,PPFCONTROL,&frob);	
return(0);
}
int port_toggleAutoFeedbit (int fd)
{
        struct ppdev_frob_struct frob;
	static char value = 0;
	value = (value)? 0: PARPORT_CONTROL_AUTOFD;
        frob.mask = PARPORT_CONTROL_AUTOFD;
        frob.val= value;	
	ioctl(fd,PPFCONTROL,&frob);
        
return(0);
}
