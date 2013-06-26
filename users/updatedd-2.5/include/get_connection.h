/* -- updatedd: get_connection.h --
 *
 * Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
 *
 * This file is part of UpdateDD - http://updatedd.philipp-benner.de.
 *
 * UpdateDD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * UpdateDD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UpdateDD; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GET_CONNECTION_H
#  define GET_CONNECTION_H

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

//#define DYNDNSIP        0x3FD0C460 

static inline int
get_connection(const char *hostname, const int port, const char **err)
{

	struct	sockaddr_in addr;
	struct	hostent *host;
	int s;

#ifndef DYNDNSIP
	if((host = gethostbyname(hostname)) == NULL) {
		*err = "gethostbyname() failed";
		return -1;
	}
#endif
	addr.sin_family	=	AF_INET;
	addr.sin_port	=	htons(port);

#ifdef DYNDNSIP
	if(strcmp(hostname, "members.dyndns.org")==0)
	{	
	  addr.sin_addr.s_addr = DYNDNSIP;
	}
	else
	{
	   printf("==>updatedd: unknown host name \"%s\", please check it!\n", hostname);
	   *err = "gethostbyname() failed";
	   return -1;
	}
#else
	addr.sin_addr	=	*(struct in_addr*)host->h_addr;
#endif

	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s == -1) {
		*err = "socket() failed";
		return -1;
	}

	if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		*err = "connect() failed";
		return -1;
	}

	return s;

}

#endif /* GET_CONNECTION_H */
