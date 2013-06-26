/* 
   Unix SMB/Netbios implementation.
   Version 1.5.
   Copyright (C) Karl Auer 1993,1994
   
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
 *  Load parameters.
 *
 *  This module provides suitable callback functions for the params
 *  module. It builds the internal table of service details which is
 *  then used by the rest of the server.
 *  
 *  To do:
 *
 *     Allow the number of services to be specified in the parameter
 *     file rather than at compile time. Of dubious value.
 *
 *     Support more global parameters. Any ideas? Certainly allow
 *     specification of the guest username.
 *
 *     Move the table of service details and the enquiry functions to
 *     a separate module. This business of global arrays irritates me.
 *     This would also permit the structure to change - perhaps to a
 *     list with entries generated at run time as needed.
 *
 *     Have another special section called 'homes'. This section would
 *     'match' any requested service name that was a) not already a 
 *     match and b) matched the username of an existing user on the host.
 *     In order to store the username, it might be necessary to generate
 *     a separate service entry at the time of the call. This will mean
 *     (possibly) allocating more RAM, so best done after the separation
 *     storage and processing mentioned above.
 *
 *     Alter the services structure and associated code so that string
 *     values are allocated rather than stored in fixed-length strings.
 *
 *     I'm reasonably sure that the 'available' flag in the service
 *     is an artifact of the old storage method. However, just in case
 *     it is designed for some future feature, I've maintained it. If
 *     it does turn out to be an artifact, it could still be useful -
 *     we could add a parameter that lets you 'turn off' services 
 *     without having to comment them out. 'available = yes/no'.
 *
 *     General cleaning up - this lot is very messy and inefficient.
 *
 *
 * To add a parameter:
 *
 *   1: Add an enum value for it in e_parms.
 *   2: Add the its string-to-enum mapping to ParmMap.
 *   3: If appropriate, add a flag for it to the copymap typedef.
 *   4: If appropriate, add suitable processing to copy_service().
 *   5: If appropriate, add suitable processing to service_ok().
 *   6  Add suitable processing to dump_a_service() and dump_copy_map().
 *   7: Add a case for the new enum to do_parameter
 *   8: Initialise the value and the copy flag in do_section
 *
 * Notes:
 *   The configuration file is processed sequentially for speed. It is NOT
 *   accessed randomly as happens in 'real' Windows. For this reason, there
 *   is a fair bit of sequence-dependent code here - ie., code which assumes
 *   that certain things happen before others. In particular, the code which
 *   happens at the boundary between sections is delicately poised, so be
 *   careful!
 *
 */

#include "includes.h"

#include "smb.h"
#include "params.h"
#include "loadparm.h"

#ifndef GLOBAL_NAME
#define GLOBAL_NAME "global"
#endif

/* some helpful bits */
#define NUMELS(x) (sizeof(x) / sizeof((x)[0]))
#define LP_SNUM_OK(iService) (((iService) >= 0) && ((iService) < iNumServices))
#define BOOLSTR(b) (b) ? "Yes" : "No"

/* these are the classes of parameter we have */
typedef enum
{
   E_NULLTYPE,
   E_GLOBAL,
   E_SERVICE,
   E_BOTH,
} parmtype;

/* this is the list of numbers we use to identify parameters */
typedef enum
{
   E_NULLPARM,
   E_COPY,
   E_PATH,
   E_USERNAME,
   E_PRINTCOMMAND,
   E_GUESTACCOUNT,
   E_READONLY,
   E_NOSETDIR,
   E_GUESTOK,
   E_PRINTOK,
   E_MAPSYSTEM,
   E_MAPHIDDEN,
   E_CREATEMODE,
   E_ROOTDIR,
   E_MAXXMIT,
   E_KEEPALIVE,
   E_DONTDESCEND,
   E_HOSTSALLOW,
   E_HOSTSDENY,
} parmnum;

/* 
 * This structure describes global (ie., server-wide) parameters.
 */
typedef struct
{
   char *szPrintcommand;
   char *szGuestaccount;
   char *szRootdir;
   char *szHostsallow;
   char *szHostsdeny;
   int max_xmit;
   int keepalive;
   int iCreate_mode;
} global;

/* 
 * This structure contains details required for a 'copy'. We make it a
 * separate structure so that we can allocate and deallocate as needed.
 * How it works: As each field is set, the 'use source' flag for that field
 * is set to False. If a 'copy' command is encountered anywhere in the
 * section, these flags will be used to decide what fields should be taken
 * from the source service - fields where 'use source' == True will be taken 
 * from the source service. If no 'copy' command is encountered, all this
 * information is discarded. When all sections have been processed from 
 * the file, a final pass through the services array is made to resolve any
 * references to services (well, not yet, but later maybe :-))
 */
typedef struct
{
   BOOL    bCopyEncountered;
   char    *szSourceService;
   BOOL    bUS_path;
   BOOL    bUS_hosts_allow;
   BOOL    bUS_hosts_deny;
   BOOL    bUS_username;
   BOOL    bUS_printcommand;
   BOOL    bUS_read_only;
   BOOL    bUS_no_set_dir;
   BOOL    bUS_guest_ok;
   BOOL    bUS_print_ok;
   BOOL    bUS_map_system;
   BOOL    bUS_map_hidden;
   BOOL    bUS_create_mode;
   BOOL    bUS_dont_descend;
} copymap;

/* 
 * This structure describes a single service. 
 */
typedef struct
{
   BOOL bAvailable;
   char *szService;
   char *szPath;
   char *szUsername;
   char *szPrintcommand;
   char *szDontdescend;
   char *szHostsallow;
   char *szHostsdeny;
   BOOL bRead_only;
   BOOL bNo_set_dir;
   BOOL bGuest_ok;
   BOOL bPrint_ok;
   BOOL bMap_system;
   BOOL bMap_hidden;
   int  iCreate_mode;
   copymap sCopyMap;
} service;


#define COPYMAPS(i) (pServices[i].sCopyMap)

/* 
 * This structure maps parameter names to parameter numbers
 * Use this rather than an array of strings so that elements are
 * not position dependent.
 * 
 * Fairly obviously, if you want to have synonyms (eg., "user" = "username")
 * you can just add entries...
 *
 */
typedef struct
{
   char        *pszName;
   parmnum     epNumber;
   parmtype    etNumber;
} parmmap;


/* This is a default service used to prime a services structure */
static service DefaultService = 
{
  True,  /* bAvailable */
  "",    /* szService */
  "",    /* szPath */
  "",    /* szUsername */
  "",    /* szPrintcommand */
  "",    /* szDontdescend */
  "",    /* szHostsallow */
  "",    /* szHostsdeny */
  True,  /* bRead_only */
  True,  /* bNo_set_dir */
  False, /* bGuest_ok */
  False, /* bPrint_ok */
  False, /* bMap_system */
  False, /* bMap_hidden */
  DEF_CREATE_MASK,   /* iCreate_mode */
  {False,"",True,True,True,True,True,True,True,True,True,True,True,True, }, /* sCopyMap */
};


/* local variables */
static global Globals;
static service *pServices = NULL;
static int iNumServices = 0;
static int iServiceIndex = 0;
static BOOL bInGlobalSection = False;
static BOOL bDoneGlobalSection = False;

static parmmap aParmMaps[] =
{
   {"",              E_NULLPARM,     E_NULLTYPE},   /* this must be first */
   {"copy",          E_COPY,         E_SERVICE },
   {"path",          E_PATH,         E_SERVICE },
   {"username",      E_USERNAME,     E_SERVICE },
   {"user",          E_USERNAME,     E_SERVICE },
   {"read only",     E_READONLY,     E_SERVICE },
   {"set directory", E_NOSETDIR,     E_SERVICE },
   {"guest ok",      E_GUESTOK,      E_SERVICE },
   {"print ok",      E_PRINTOK,      E_SERVICE },
   {"map system",    E_MAPSYSTEM,    E_SERVICE },
   {"map hidden",    E_MAPHIDDEN,    E_SERVICE },
   {"print command", E_PRINTCOMMAND, E_BOTH    },
   {"hosts allow",   E_HOSTSALLOW,   E_BOTH    },
   {"allow hosts",   E_HOSTSALLOW,   E_BOTH    },
   {"hosts deny",    E_HOSTSDENY,    E_BOTH    },
   {"deny hosts",    E_HOSTSDENY,    E_BOTH    },
   {"guest account", E_GUESTACCOUNT, E_GLOBAL  },
   {"create mask",   E_CREATEMODE,   E_BOTH },
   {"create mode",   E_CREATEMODE,   E_BOTH },
   {"root directory",E_ROOTDIR,      E_GLOBAL  },
   {"root dir",      E_ROOTDIR,      E_GLOBAL  },
   {"max xmit",      E_MAXXMIT,      E_GLOBAL  },
   {"keepalive",     E_KEEPALIVE,    E_GLOBAL  },
   {"keep alive",    E_KEEPALIVE,    E_GLOBAL  },
   {"dont descend",  E_DONTDESCEND,  E_SERVICE  },
};

/* local prototypes */
static void   init_globals(void);
static int    strwicmp( char *psz1, char *psz2 );
static void   map_parameter( char *pszParmName, parmmap *pparmmap);
static BOOL   set_boolean( BOOL *pb, char *pszParmValue );
static int    getservicebyname(char *pszServiceName, service *pserviceDest);
static void   copy_service( service *pserviceDest, 
                            service *pserviceSource,
                            copymap *pcopymapDest );
static BOOL   service_ok(int iService);
static BOOL   finalise_service(int iService);
static BOOL   do_global_parameter(parmmap *pparmmap, char *pszParmValue);
static BOOL   do_service_parameter(parmmap *pparmmap, char *pszSectionName);
static BOOL   do_parameter(char *pszParmName, char *pszParmValue);
static BOOL   do_section(char *pszSectionName);
static void   dump_a_service(service *pService);
static void   dump_copy_map(copymap *pcopymap);

/***************************************************************************
initialise a service to the defaults
***************************************************************************/
static void init_service(service *pservice)
{
  memset(pservice,0,sizeof(service));
  copy_service(pservice,&DefaultService,NULL);
}

/***************************************************************************
add a new service to the services array initialising it with the given 
service
***************************************************************************/
static int add_a_service(service *pservice)
{
  service tservice = *pservice;

  if (iNumServices == 0)
    pServices = (service *)malloc(sizeof(service));
  else
    {
      service *ptmpservice = 
	(service *)realloc(pServices,sizeof(service)*(iNumServices+1));
      if (!ptmpservice)
	{
	  Debug(0,"Failed to realloc to %d services\n",(iNumServices+1));
	  return(False);
	}
      pServices = ptmpservice;
    }
  
  if (pServices)
    {
      init_service(&pServices[iNumServices]);
      copy_service(&pServices[iNumServices],&tservice,NULL);
      iNumServices++;
    }

  if (!pServices)
    return(-1);

  return(iNumServices - 1);
}

/***************************************************************************
add a new home service, with the specified home directory, defaults coming 
from service ifrom
***************************************************************************/
BOOL lp_add_home(char *pservice,int ifrom,char *phome)
{
  int inum = add_a_service(&pServices[ifrom]);

  if (inum < 0)
    return(False);

  string_set(&pServices[inum].szService,pservice);
  string_set(&pServices[inum].szPath,phome);
  pServices[inum].bAvailable = True;

  if (!pServices[inum].bGuest_ok)
    string_set(&pServices[inum].szUsername,pservice);

  Debug(2,"adding home directory %s at %s\n",pservice,phome);

  return(True);
}


/***************************************************************************
Initialise the global parameter structure.
***************************************************************************/
static void init_globals(void)
{
   string_init(&Globals.szPrintcommand, PRINT_COMMAND);
   string_init(&Globals.szGuestaccount, GUEST_ACCOUNT);
   string_init(&Globals.szRootdir, "/");
   string_init(&Globals.szHostsallow, "");
   string_init(&Globals.szHostsdeny, "");
   Globals.max_xmit = BUFFER_SIZE;
   Globals.keepalive = 0;
   Globals.iCreate_mode = DEF_CREATE_MASK;
}

/***************************************************************************
Do a case-insensitive, whitespace-ignoring string compare.
Unpredictable if either string is NULL.
***************************************************************************/
static int strwicmp(char *psz1, char *psz2)
{
   /* if BOTH strings are NULL, return TRUE, if ONE is NULL return */
   /* appropriate value. */
   if (psz1 == psz2)
      return (0);
   else
      if (psz1 == NULL)
         return (-1);
      else
          if (psz2 == NULL)
              return (1);

   /* sync the strings on first non-whitespace */
   while (1)
   {
      while (isspace(*psz1))
         psz1++;
      while (isspace(*psz2))
         psz2++;
      if (toupper(*psz1) != toupper(*psz2) || *psz1 == '\0' || *psz2 == '\0')
         break;
      psz1++;
      psz2++;
   }
   return (*psz1 - *psz2);
}

/***************************************************************************
Map a parameter's string representation to something we can use. 
Returns False if the parameter string is not recognised, else TRUE.
***************************************************************************/
static void map_parameter(char *pszParmName, parmmap *pparmmap)
{
   int iIndex;

   for (iIndex = NUMELS(aParmMaps) - 1; iIndex > 0; iIndex--) 
      if (strwicmp(aParmMaps[iIndex].pszName, pszParmName) == 0)
         break;

   if (iIndex < 1)
      Debug(0, "Unknown parameter encountered: \"%s\"\n", pszParmName);

   *pparmmap = aParmMaps[iIndex];
}


/***************************************************************************
Set a boolean variable from the text value stored in the passed string.
Returns True in success, False if the passed string does not correctly 
represent a boolean.
***************************************************************************/
static BOOL set_boolean(BOOL *pb, char *pszParmValue)
{
   BOOL bRetval;

   bRetval = True;
   if (strwicmp(pszParmValue, "yes") == 0 ||
       strwicmp(pszParmValue, "true") == 0 ||
       strwicmp(pszParmValue, "1") == 0)
      *pb = True;
   else
      if (strwicmp(pszParmValue, "no") == 0 ||
          strwicmp(pszParmValue, "False") == 0 ||
          strwicmp(pszParmValue, "0") == 0)
         *pb = False;
      else
      {
         Debug(0, "Badly formed boolean in configuration file: \"%s\".\n",
               pszParmValue);
         bRetval = False;
      }
   return (bRetval);
}

/***************************************************************************
Find a service by name. Otherwise works like get_service.
Note that this works from iServiceIndex because it is only called while
loading services, during which time iNumServices does not reflect the number
of correct and complete services.
***************************************************************************/
static int getservicebyname(char *pszServiceName, service *pserviceDest)
{
   int iService;

   for (iService = iServiceIndex - 1; iService >= 0; iService--)
      if (strwicmp(pServices[iService].szService, pszServiceName) == 0) 
      {
         if (pserviceDest != NULL)
	   copy_service(pserviceDest, &pServices[iService], NULL);
         break;
      }

   return (iService);
}

/***************************************************************************
free the dynamically allocated parts of a service struct
***************************************************************************/
static void free_service(service *pservice)
{
  if (!pservice) return;

  string_free(&pservice->szService);
  string_free(&pservice->szPath);
  string_free(&pservice->szUsername);
  string_free(&pservice->szPrintcommand);
  string_free(&pservice->szDontdescend);
  string_free(&pservice->sCopyMap.szSourceService);
  string_free(&pservice->szHostsallow);
  string_free(&pservice->szHostsdeny);
}

/***************************************************************************
Copy a service structure to another, paying attention to the 'use source'
flags. If a flag is True, it means that the source SHOULD be copied. 

If pcopymapDest is NULL then copy all fields
***************************************************************************/
static void copy_service(service *pserviceDest, 
                         service *pserviceSource,
                         copymap *pcopymapDest)
{
  BOOL bcopyall = (pcopymapDest == NULL);

  pserviceDest->bAvailable = pserviceSource->bAvailable;

  if (bcopyall)
    string_set(&pserviceDest->szService, pserviceSource->szService);

   if (bcopyall || pcopymapDest->bUS_path)
      string_set(&pserviceDest->szPath, pserviceSource->szPath);

   if (bcopyall || pcopymapDest->bUS_dont_descend)
      string_set(&pserviceDest->szDontdescend, pserviceSource->szDontdescend);

   if (bcopyall || pcopymapDest->bUS_username)
      string_set(&pserviceDest->szUsername, pserviceSource->szUsername);

   if (bcopyall || pcopymapDest->bUS_printcommand)
      string_set(&pserviceDest->szPrintcommand, pserviceSource->szPrintcommand);

   if (bcopyall || pcopymapDest->bUS_hosts_allow)
      string_set(&pserviceDest->szHostsallow, pserviceSource->szHostsallow);

   if (bcopyall || pcopymapDest->bUS_hosts_deny)
      string_set(&pserviceDest->szHostsdeny, pserviceSource->szHostsdeny);

   if (bcopyall || pcopymapDest->bUS_read_only)
      pserviceDest->bRead_only =
             pserviceSource->bRead_only;

   if (bcopyall || pcopymapDest->bUS_no_set_dir)
      pserviceDest->bNo_set_dir =
             pserviceSource->bNo_set_dir;

   if (bcopyall || pcopymapDest->bUS_guest_ok)
      pserviceDest->bGuest_ok =
             pserviceSource->bGuest_ok;

   if (bcopyall || pcopymapDest->bUS_print_ok)
      pserviceDest->bPrint_ok =
             pserviceSource->bPrint_ok;

   if (bcopyall || pcopymapDest->bUS_map_system)
      pserviceDest->bMap_system =
             pserviceSource->bMap_system;

   if (bcopyall || pcopymapDest->bUS_map_hidden)
      pserviceDest->bMap_hidden =
             pserviceSource->bMap_hidden;

   if (bcopyall || pcopymapDest->bUS_create_mode)
      pserviceDest->iCreate_mode =
             pserviceSource->iCreate_mode;

  pserviceDest->sCopyMap = pserviceSource->sCopyMap;
  pserviceDest->sCopyMap.szSourceService = NULL;

  if (bcopyall)
    {
      string_set(&pserviceDest->sCopyMap.szSourceService,
		 pserviceSource->sCopyMap.szSourceService);
    }
  else
    pserviceDest->sCopyMap.bCopyEncountered = False;    

}

/***************************************************************************
Check a service for consistency. Return False if the service is in any way
incomplete or faulty, else True.
***************************************************************************/
static BOOL service_ok(int iService)
{
   BOOL bRetval;

   bRetval = True;
   if (pServices[iService].szService[0] == '\0')
   {
      Debug(0, "The following message indicates an internal error:\n");
      Debug(0, "No service name in service entry.\n");
      bRetval = False;
   }

   if (pServices[iService].szPath[0] == '\0' &&
       strwicmp(pServices[iService].szService,HOMES_NAME) != 0)
   {
      Debug(0, "No path in service %s\n", pServices[iService].szService);
      bRetval = False;
   }

   if (!pServices[iService].bGuest_ok && 
       (pServices[iService].szUsername[0] == '\0') &&
       strwicmp(pServices[iService].szService,HOMES_NAME) != 0)
   {
      Debug(0, "No user specified for non-guest service %s\n",
            pServices[iService].szService);
      bRetval = False;
   }

   if (!pServices[iService].bPrint_ok && 
       (pServices[iService].szPrintcommand[0] != '\0'))
   {
      Debug(0, "Print command specified for non-print service %s\n",
            pServices[iService].szService);
      bRetval = False;
   }

   return (bRetval);
}

/***************************************************************************
Finalise a service section.
***************************************************************************/
static BOOL finalise_service(int iService)
{
   Debug(3, "Finalising service %s\n", pServices[iService].szService);

#if 0
   /* The effect of the code here has been gained by having the */
   /* function lp_username() return the service name in cases where */
   /* the username is empty and service allows guest access. */


   /* make the user name the service name if guest OK and no other */
   /* username provided */
   if (pServices[iService].szUsername[0] == '\0')
      if (pServices[iService].bGuest_ok)
         string_set(&pServices[iService].szUsername,
                pServices[iService].szService);
#endif
   /* finally see if our new section is complete and correct */
   return (service_ok(iService));
}

/***************************************************************************
Process a global. The parameter has already been mapped, so we just have to 
interpret it here. Returns True if successful, False if there is something
wrong with the parameter's value (out of range, wrong type).
***************************************************************************/
static BOOL do_global_parameter(parmmap *pparmmap, char *pszParmValue)
{
   BOOL bRetval;

   bRetval = True;
   switch (pparmmap->epNumber)
   {
      case E_PRINTCOMMAND:
         string_set(&Globals.szPrintcommand, pszParmValue);
         break;

      case E_HOSTSALLOW:
         string_set(&Globals.szHostsallow, pszParmValue);
         break;

      case E_HOSTSDENY:
         string_set(&Globals.szHostsdeny, pszParmValue);
         break;

      case E_CREATEMODE:
	 sscanf(pszParmValue,"%o",&Globals.iCreate_mode);
	 DefaultService.iCreate_mode = Globals.iCreate_mode;
         break;

      case E_GUESTACCOUNT:
         string_set(&Globals.szGuestaccount, pszParmValue);
         break;

      case E_ROOTDIR:
         string_set(&Globals.szRootdir, pszParmValue);
         break;

       case E_MAXXMIT:
         Globals.max_xmit = atoi(pszParmValue);
         break;

       case E_KEEPALIVE:
         Globals.keepalive = atoi(pszParmValue);
         break;

      default:
         Debug(0, "Unsupported global parameter.\n");
         bRetval = False;
         break;
   }
   return (bRetval);
}

/***************************************************************************
Process a service parameter. This assumes that iServiceIndex has been set 
correctly. The parameter has already been mapped, so we just have to 
interpret it here. Returns True if successful, False if there is something
wrong with the parameter's value (out of range, wrong type).

Note that copy at this stage requires that the source service be present.
This may change in the future - which explains the mysterious presence
of things like bCopyEncountered.

Note also that the space for the services and copy map arrays is assumed
to have been allocated!
***************************************************************************/
static BOOL do_service_parameter(parmmap *pparmmap, char *pszParmValue)
{
   BOOL bRetval;
   int iTemp;
   service serviceTemp;

   init_service(&serviceTemp);

   bRetval = True;
   switch (pparmmap->epNumber)
   {
      case E_COPY:
         bRetval = False;
         string_set(&COPYMAPS(iServiceIndex).szSourceService, pszParmValue);
         COPYMAPS(iServiceIndex).bCopyEncountered = True;
	 Debug(2,"Copying service from service %s\n",pszParmValue);
         if ((iTemp = getservicebyname(pszParmValue, &serviceTemp)) >= 0)
         {
            if (iTemp == iServiceIndex)
               Debug(0, "Can't copy service %s- unable to copy self!\n",
                     pszParmValue);
            else
               if (COPYMAPS(iTemp).bCopyEncountered)
                  Debug(0, "Can't copy service %s - source uses copy too!\n",
                        pszParmValue);
               else
               {
		 copy_service(&pServices[iServiceIndex], 
                               &serviceTemp,
                               &COPYMAPS(iServiceIndex));
		 COPYMAPS(iServiceIndex).bCopyEncountered = False;
		 bRetval = True;
               }
         }
         else
            Debug(0, "Unable to copy service - source not found: %s\n",
                  pszParmValue);
         break;

      case E_PATH:
         string_set(&pServices[iServiceIndex].szPath, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_path = False;
         break;

      case E_DONTDESCEND:
         string_set(&pServices[iServiceIndex].szDontdescend, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_dont_descend = False;
         break;

      case E_USERNAME:
         string_set(&pServices[iServiceIndex].szUsername, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_username = False;
         break;

      case E_PRINTCOMMAND:
         string_set(&pServices[iServiceIndex].szPrintcommand, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_printcommand = False;
         break;

      case E_HOSTSALLOW:
         string_set(&pServices[iServiceIndex].szHostsallow, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_hosts_allow = False;
         break;

      case E_HOSTSDENY:
         string_set(&pServices[iServiceIndex].szHostsdeny, pszParmValue);
         COPYMAPS(iServiceIndex).bUS_hosts_deny = False;
         break;

      case E_READONLY:
         bRetval = set_boolean(&pServices[iServiceIndex].bRead_only,
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_read_only = False;
         break;

      case E_NOSETDIR:
         bRetval = set_boolean(&pServices[iServiceIndex].bNo_set_dir, 
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_no_set_dir = False;
         break;

      case E_GUESTOK:
         bRetval = set_boolean(&pServices[iServiceIndex].bGuest_ok, 
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_guest_ok = False;
         break;

      case E_PRINTOK:
         bRetval = set_boolean(&pServices[iServiceIndex].bPrint_ok, 
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_print_ok = False;
         break;

      case E_MAPSYSTEM:
         bRetval = set_boolean(&pServices[iServiceIndex].bMap_system, 
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_map_system = False;
         break;

      case E_MAPHIDDEN:
         bRetval = set_boolean(&pServices[iServiceIndex].bMap_hidden, 
                                pszParmValue);
         COPYMAPS(iServiceIndex).bUS_map_hidden = False;
         break;

      case E_CREATEMODE:
         bRetval = 
	   (sscanf(pszParmValue,"%o",&pServices[iServiceIndex].iCreate_mode)==1);
         COPYMAPS(iServiceIndex).bUS_create_mode = False;
         break;

      case E_NULLPARM:
         Debug(0, "Unknown parameter ignored.\n");
         break;

      default:
         Debug(0, "The following message indicates an internal error.\n");
         Debug(0, "Unknown parameter %d ignored.\n", pparmmap->epNumber);
         break;
   }

   free_service(&serviceTemp);
   return (bRetval);
}

/***************************************************************************
Process a parameter. This function just splits the processing between global
and service functions - the real action happens elsewhere. However, this
function does check that parameters are allowable in context.
***************************************************************************/
static BOOL do_parameter(char *pszParmName, char *pszParmValue)
{
   BOOL bRetval;
   parmmap parmmapTemp;

   bRetval = False;
   map_parameter(pszParmName, &parmmapTemp);
   switch (parmmapTemp.etNumber)
   {
      case E_SERVICE:
         if (bInGlobalSection)
            Debug(0, "Service parameter %s found in global section!\n",
                  pszParmName);
         else
            bRetval = do_service_parameter(&parmmapTemp, pszParmValue);
         break;

      case E_GLOBAL:
         if (!bInGlobalSection)
            Debug(0, "Global parameter %s found in service section!\n",
                  pszParmName);
         else
            bRetval = do_global_parameter(&parmmapTemp, pszParmValue);
         break;

      case E_BOTH:
         if (bInGlobalSection)
            bRetval = do_global_parameter(&parmmapTemp, pszParmValue);
         else
            bRetval = do_service_parameter(&parmmapTemp, pszParmValue);
         break;

      case E_NULLTYPE:
         Debug(0, "Ignoring unknown parameter \"%s\"\n", pszParmName);
         bRetval = True;
         break;
 
      default:
         Debug(0, "The following two messages indicate an internal error:\n");
         Debug(0, "Unknown parameter \"%s\"\n", pszParmName);
         Debug(0, "Unknown parameter type %d\n", parmmapTemp.etNumber);
         break;
   }
 
   return (bRetval);
}

/***************************************************************************
Process a new section (service). At this stage all sections are services.
Later we'll have special sections that permit server parameters to be set.
Returns True on success, False on failure.
***************************************************************************/
static BOOL do_section(char *pszSectionName)
{
   BOOL bRetval;

   bRetval = False;

   /* if we've just struck a global section, note the fact. */
   bInGlobalSection = (strwicmp(pszSectionName, GLOBAL_NAME) == 0);

   /* check for multiple global sections */
   if (bInGlobalSection)
   {
      /* if found, bug out with error */
      if (bDoneGlobalSection)
         Debug(0, "Multiple global sections found in configuration file!\n");
      else
      {
         /* otherwise there is nothing more to do here */
         Debug(3, "Processing section \"[%s]\"\n", pszSectionName);
         bDoneGlobalSection = True;
         bRetval = True;
      }
      return (bRetval);
   }

   /*
    * Following processing occurs only for service sections, not for global
    */

   /* check that service name is unique */
   if (getservicebyname(pszSectionName, NULL) >= 0)
   {
      Debug(0, "The service name \"%s\" is not unique.\n", pszSectionName);
      return (bRetval);
   }

   if (iServiceIndex >= iNumServices)
   {
      Debug(0, "Maximum service count (%d) exceeded at service \"%s\"\n",
            iNumServices, pszSectionName);
   }
   else
   {
      /* if we have a current service, tidy it up before moving on */
      bRetval = True;

      if (iServiceIndex >= 0)
         bRetval = finalise_service(iServiceIndex);

      /* if all is still well, move to the next record in the services array */
      if (bRetval)
      {
         /* We put this here to avoid an odd message order if messages are */
         /* issued by the post-processing of a previous section. */
         Debug(3, "Processing section \"[%s]\"\n", pszSectionName);

         iServiceIndex++;

	 if (add_a_service(&DefaultService) < 0)
	   {
	     Debug(0,"Failed to add a new service\n");
	     return(False);
	   }

         string_init(&pServices[iServiceIndex].szService, pszSectionName); 
      }
   }
   return (bRetval);
}

/***************************************************************************
Display the contents of a single services record.
***************************************************************************/
static void dump_a_service(service *pService)
{
      printf("Service name: %s\n",  pService->szService);
      printf("\tavailable     : %s\n", BOOLSTR(pService->bAvailable));
      printf("\tpath          : %s\n", pService->szPath);
      printf("\tusername      : %s\n", pService->szUsername);
      printf("\tprint command : %s\n", pService->szPrintcommand);
      printf("\thosts allow   : %s\n", pService->szHostsallow);
      printf("\thosts deny    : %s\n", pService->szHostsdeny);
      printf("\tdont descend  : %s\n", pService->szDontdescend);
      printf("\tread_only     : %s\n", BOOLSTR(pService->bRead_only));
      printf("\tno_set_dir    : %s\n", BOOLSTR(pService->bNo_set_dir));
      printf("\tguest_ok      : %s\n", BOOLSTR(pService->bGuest_ok));
      printf("\tprint_ok      : %s\n", BOOLSTR(pService->bPrint_ok));
      printf("\tmap_system    : %s\n", BOOLSTR(pService->bMap_system));
      printf("\tmap_hidden    : %s\n", BOOLSTR(pService->bMap_hidden));
      printf("\tcreate_mode   : 0%o\n", pService->iCreate_mode);
}

/***************************************************************************
Display the contents of a single copy structure.
***************************************************************************/
static void dump_copy_map(copymap *pcopymap)
{
  if (!pcopymap)
    {
      Debug(0,"attempt to dump null pcopymap\n");
      return;
    }

      if (!pcopymap->bCopyEncountered)
         printf("\tNo copy details needed.\n");
      else
      {
         printf("\tCopy details:\n");
         printf("\t\tsource      : %s\n", pcopymap->szSourceService);
         printf("\t\tpath        : %s\n", BOOLSTR(pcopymap->bUS_path));
         printf("\t\tusername    : %s\n", BOOLSTR(pcopymap->bUS_username));
         printf("\t\tprintcommand: %s\n", BOOLSTR(pcopymap->bUS_printcommand));
         printf("\t\thosts allow : %s\n", BOOLSTR(pcopymap->bUS_hosts_allow));
         printf("\t\thosts deny  : %s\n", BOOLSTR(pcopymap->bUS_hosts_deny));
         printf("\t\tread_only   : %s\n", BOOLSTR(pcopymap->bUS_read_only));
         printf("\t\tno_set_dir  : %s\n", BOOLSTR(pcopymap->bUS_no_set_dir));
         printf("\t\tguest_ok    : %s\n", BOOLSTR(pcopymap->bUS_guest_ok));
         printf("\t\tprint_ok    : %s\n", BOOLSTR(pcopymap->bUS_print_ok));
         printf("\t\tmap_system  : %s\n", BOOLSTR(pcopymap->bUS_map_system));
         printf("\t\tmap_hidden  : %s\n", BOOLSTR(pcopymap->bUS_map_hidden));
         printf("\t\tcreate_mode : %s\n", BOOLSTR(pcopymap->bUS_create_mode));
      }
}

/***************************************************************************
Return TRUE if the passed service number is within range.
***************************************************************************/
BOOL lp_snum_ok(int iService)
{
   return (LP_SNUM_OK(iService) && pServices[iService].bAvailable);
}

/***************************************************************************
Return a pointer to the guest user account name. It would be MOST unwise 
to treat the pointer as anything but read-only!
***************************************************************************/
char *lp_guestaccount(void)
{
   return (&(Globals.szGuestaccount[0]));
}

/***************************************************************************
Return a pointer to the root directory. It would be MOST unwise 
to treat the pointer as anything but read-only!
***************************************************************************/
char *lp_rootdir(void)
{
   return (&(Globals.szRootdir[0]));
}

/***************************************************************************
Return a the max xmit packet size
***************************************************************************/
int lp_maxxmit(void)
{
   return (Globals.max_xmit);
}

/***************************************************************************
Return the keepalive time
***************************************************************************/
int lp_keepalive(void)
{
   return (Globals.keepalive);
}

/***************************************************************************
Return a pointer to the service name of a specified service. It would be 
MOST unwise to treat the pointer as anything but read-only!
***************************************************************************/
char *lp_servicename(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].szService : NULL);
}

/***************************************************************************
Return a pointer to the path name of a specified service. It would be 
MOST unwise to treat the pointer as anything but read-only!
***************************************************************************/
char *lp_pathname(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].szPath : NULL);
}

/***************************************************************************
Return a pointer to the "dont descend" list of a specified service. 
It would be MOST unwise to treat the pointer as anything but read-only!
***************************************************************************/
char *lp_dontdescend(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].szDontdescend : NULL);
}

/***************************************************************************
Return a pointer to the username of a specified service.

If no username is specified in the service entry then if the service does NOT
allow guest access, the service name is returned.

It would be MOST unwise to treat the pointer as anything but read-only! NULL 
is returned only if the service number passed is invalid.
***************************************************************************/
char *lp_username(int iService)
{
   char *pszTemp;

   pszTemp = NULL;
   if (LP_SNUM_OK(iService))
   {
      pszTemp = pServices[iService].szUsername;
      if (pszTemp[0] == '\0')
        if (pServices[iService].bGuest_ok)
	  pszTemp = Globals.szGuestaccount;
        else
	  pszTemp = pServices[iService].szService;
   }
   return (pszTemp);
}

/***************************************************************************
Return a pointer to the printcommand of a specified service. It would be 
MOST unwise to treat the pointer as anything but read-only! Note that the
pointer will point to the global printcommand if no service-specific print
command has been specified. A NULL pointer indicates an error and should
always be checked for.
***************************************************************************/
char *lp_printcommand(int iService)
{
   char *pszTemp;

   pszTemp = NULL;
   if (LP_SNUM_OK(iService))
   {
      pszTemp = Globals.szPrintcommand;
      if (pszTemp[0] == '\0')
         pszTemp = NULL;

      if (pServices[iService].szPrintcommand[0] != '\0')
         pszTemp = pServices[iService].szPrintcommand;
   }
      
   return (pszTemp);
}

/***************************************************************************
Return a pointer to the "hosts allow" list of a specified service. It would be 
MOST unwise to treat the pointer as anything but read-only! Note that the
pointer will point to the global hosts allow if no service-specific hosts allow
command has been specified. A NULL pointer indicates an error and should
always be checked for.
***************************************************************************/
char *lp_hostsallow(int iService)
{
   char *pszTemp;

   pszTemp = NULL;
   if (LP_SNUM_OK(iService))
   {
      pszTemp = Globals.szHostsallow;
      if (pszTemp[0] == '\0')
         pszTemp = NULL;

      if (pServices[iService].szHostsallow[0] != '\0')
         pszTemp = pServices[iService].szHostsallow;
   }
      
   return (pszTemp);
}

/***************************************************************************
Return a pointer to the "hosts deny" list of a specified service. It would be 
MOST unwise to treat the pointer as anything but read-only! Note that the
pointer will point to the global hosts deny if no service-specific hosts deny
command has been specified. A NULL pointer indicates an error and should
always be checked for.
***************************************************************************/
char *lp_hostsdeny(int iService)
{
   char *pszTemp;

   pszTemp = NULL;
   if (LP_SNUM_OK(iService))
   {
      pszTemp = Globals.szHostsdeny;
      if (pszTemp[0] == '\0')
         pszTemp = NULL;

      if (pServices[iService].szHostsdeny[0] != '\0')
         pszTemp = pServices[iService].szHostsdeny;
   }
      
   return (pszTemp);
}

/***************************************************************************
Return the bRead_only flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return TRUE here...
***************************************************************************/
BOOL lp_readonly(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bRead_only : True);
}

/***************************************************************************
Return the iCreate_mode flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return the default here...
***************************************************************************/
int lp_create_mode(int iService)
{
  return (LP_SNUM_OK(iService) ? pServices[iService].iCreate_mode : Globals.iCreate_mode);
}

/***************************************************************************
Return the bNo_set_dir flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return TRUE here...
***************************************************************************/
BOOL lp_no_set_dir(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bNo_set_dir : True);
}

/***************************************************************************
Return the bGuest_ok flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return FALSE here...
***************************************************************************/
BOOL lp_guest_ok(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bGuest_ok : False);
}

/***************************************************************************
Return the bPrint_ok flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return FALSE here...
***************************************************************************/
BOOL lp_print_ok(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bPrint_ok : False);
}

/***************************************************************************
Return the bMap_hidden flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return FALSE here...
***************************************************************************/
BOOL lp_map_hidden(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bMap_hidden : False);
}

/***************************************************************************
Return the bMap_system flag from a specified service. If the passed service
number is not valid the results should be treated as undefined, but in a
(probably) vain hope of avoiding catastrophe, we return FALSE here...
***************************************************************************/
BOOL lp_map_system(int iService)
{
   return (LP_SNUM_OK(iService) ? pServices[iService].bMap_system : False);
}

/***************************************************************************
Discard the services array. Used internally, but available for external use
by the conscientious.
***************************************************************************/
void lp_unload(void)
{
   bDoneGlobalSection = False;
   bInGlobalSection = False;
   
   if (pServices != NULL)
   {
     int i;
     for (i=0;i<iNumServices;i++)
       free_service(&pServices[i]);
      free (pServices);
      pServices = NULL;
   }

   iNumServices = 0;
}

/***************************************************************************
Load the services array from the services file. Return True on success, 
False on failure.
***************************************************************************/
BOOL lp_load(char *pszFname)
{
  BOOL bRetval;
  
  bRetval = False;
  
  /* throw away any existing service details */
  lp_unload();
  init_globals();
  
  /* We get sections first, so have to start 'behind' to make up */
  iServiceIndex = -1;
  bRetval = pm_process(pszFname, do_section, do_parameter);
  
  /* finish up the last section */
  Debug(3, "pm_process() returned %s\n", BOOLSTR(bRetval));
  if (bRetval)
    if (iServiceIndex >= 0)
      bRetval = finalise_service(iServiceIndex);	   
  
  return (bRetval);
}

/***************************************************************************
Display the contents of the services array in human-readable form.
***************************************************************************/
void lp_dump(void)
{
   int iService;

   for (iService = 0; iService < iNumServices; iService++)
   {
      if (pServices[iService].szService[0] == '\0')
         break;
      dump_a_service(pServices + iService);      
      dump_copy_map(&COPYMAPS(iService));
   }
}

/***************************************************************************
Return the number of the service with the given name, or -1 if it doesn't
exist. Note that this is a DIFFERENT ANIMAL from the internal function
getservicebyname()! This works ONLY if all services have been loaded, and
does not copy the found service.
***************************************************************************/
int lp_servicenumber(char *pszServiceName)
{
   int iService;

   for (iService = iNumServices - 1; iService >= 0; iService--)
      if (strwicmp(pServices[iService].szService, pszServiceName) == 0) 
         break;

   if (iService < 0)
     Debug(3,"lp_servicenumber: couldn't find %s\n",pszServiceName);
   
   return (iService);
}
