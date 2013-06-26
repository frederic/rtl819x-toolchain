/****************************************************************************
#	 	spcaview: Sdl video recorder and viewer with sound.         #
#This package work with the spca5xx based webcam with the raw jpeg feature. #
#All the decoding is in user space with the help of libjpeg.                #
#.                                                                          #
# 		Copyright (C) 2003 2004 2005 Michel Xhaard                  #
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
****************************************************************************/
 
#ifndef TCPUTILS_H
#define TCPUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXCONNECT 10
#define SERVEUR_PORT 7070
#define CLIENT_PORT 7071
#define SERVEUR_ADR "192.168.0.179"
typedef struct sockaddr_in SA;
int 
open_sock(int port);
int 
open_clientsock(char * address, int port);
int 
read_sock(int sockhandle, unsigned char* buf, int length);
int 
write_sock(int sockhandle, unsigned char* buf, int length);
void 
close_sock(int sockhandle);
int 
reportip( char *src, char *ip, unsigned short *port);
#endif /* TCPUTILS_H*/
