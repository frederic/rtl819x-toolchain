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
 * Testbed for loadparm.c/params.c
 *
 * This module simply loads a specified configuration file and
 * if successful, dumps it's contents to stdout. Note that the
 * operation is performed with DEBUGLEVEL at 3.
 *
 * Useful for a quick 'syntax check' of a configuration file.
 *
 */

#include "includes.h"
#include "smb.h"
#include "params.h"
#include "loadparm.h"

/* these live in util.c */
extern FILE *dbf;
extern int DEBUGLEVEL;

int main(int argc, char *argv[])
{
   if (argc < 2)
      printf("Please specify a services file\n");
   else
   {
      dbf = fopen("test.log", "w");
      if (dbf == NULL)
         printf("Unable to open logfile.\n");
      else
      {
         DEBUGLEVEL = 3;
         if (!lp_load(argv[1]))
            printf("Error loading services.\n");
         else
	   {
	     printf("Loaded services file OK.\n");
	     {
	       int iHomeService;
	       if ((iHomeService = lp_servicenumber(HOMES_NAME)) >= 0)
		 lp_add_home("fred",iHomeService,"/home/fred");
	     }
	     lp_dump();
	   }
         fclose(dbf);
	 if (argc == 4)
	   {
	     struct from_host f;
	     int s;
	     f.name = argv[2];
	     f.addr = argv[3];

	     /* this is totally ugly, a real `quick' hack */
	     for (s=0;s<1000;s++)
	       if (VALID_SNUM(s))
		 {		 
		   if (allow_access(lp_hostsdeny(s),lp_hostsallow(s),&f))
		     {
		       printf("Allow connection from %s (%s) to %s\n",
			     f.name,f.addr,lp_servicename(s));
		     }
		   else
		     {
		       printf("Deny connection from %s (%s) to %s\n",
			     f.name,f.addr,lp_servicename(s));
		     }
		 }
	   }
       }
   }
   return (0);
}

