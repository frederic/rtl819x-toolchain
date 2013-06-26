/* 
   Unix SMB/Netbios implementation.
   Version 1.5.
   Copyright (C) Karl Auer 1993, 1994
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 *
 * Prototypes etc for loadparm.c.
 *
 */
#ifndef _LOADPARM_H
#define _LOADPARM_H

#include "smb.h"

extern BOOL lp_snum_ok(int iService);
extern char *lp_guestaccount(void);
extern char *lp_rootdir(void);
extern int  lp_maxxmit(void);
extern int  lp_keepalive(void);
extern char *lp_servicename(int iService);
extern char *lp_pathname(int iService);
extern char *lp_username(int iService);
extern char *lp_printcommand(int iService);
extern char *lp_hostsallow(int iService);
extern char *lp_hostsdeny(int iService);
extern char *lp_dontdescend(int iService);
extern BOOL lp_readonly(int iService);
extern BOOL lp_no_set_dir(int iService);
extern BOOL lp_guest_ok(int iService);
extern BOOL lp_print_ok(int iService);
extern BOOL lp_map_hidden(int iService);
extern BOOL lp_map_system(int iService);
extern void lp_unload(void);
extern BOOL lp_load(char *pszFname);
extern void lp_dump(void);
extern int  lp_servicenumber(char *pszServiceName);
extern int lp_create_mode(int iService);
extern BOOL lp_add_home(char *pservice,int ifrom,char *phome);
#endif
