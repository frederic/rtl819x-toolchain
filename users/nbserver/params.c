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

/**************************************************************************
PARAMS.C

Copyright (C) 1990, 1991, 1992, 1993, 1994 Karl Auer

This module provides for streamlines retrieval of information from a
Windows-like parameter files. There is a function which will search for
all sections in the file and call a specified function with each. There is
a similar function which will call a specified function for all parameters 
in a section. The idea is that you pass the addresses of suitable functions
to a single function in this module which will then enumerate all sections, 
and within each section all parameters, to your program. 

Parameter files contain text lines (newline delimited) which consist of
either a section name in square brackets or a parameter name, delimited
from the parameter value by an equals sign. Blank lines or lines where the
first non-whitespace character is a colon are ignored. All whitespace in
section names and parameter names is compressed to single spaces. Leading 
and trailing whitespace on parameter names and parameter values is stripped.

Only the first equals sign in a parameter line is significant - parameter
values may contain equals signs, square brackets and semicolons. Internal
whitespace is retained in parameter values. Parameter names may not start 
with a square bracket, an equals sign or a semicolon, for obvious reasons. 

A sample parameter file might look like this:

[things]
this=1
that=2
[other things]
the other = 3

**************************************************************************/

#include "includes.h"

#include "smb.h"
#include "params.h"

/* local variable pointing to passed filename */
static char *pszParmFile = NULL;

/* local prototypes */
static BOOL enumerate_parameters(FILE *infile, PM_PARMFUNC pfunc);
static BOOL enumerate_sections(FILE *infile, 
			       PM_SECFUNC sfunc, PM_PARMFUNC pfunc);

/* prototypes for local toolbox functions */
static void trimleft(char *psz);
static void trimright(char *psz);
static void closestr(char *psz, int iStart, int nCount);
static void collapse_spaces(char *psz);
static int  firstnonwhite(char *psz);

/**************************************************************************
Identifies all parameters in the current section, calls the parameter
function for each. Ignores comment lines, stops and backs up in file when
a section is encountered. Returns True on success, False on error.
**************************************************************************/
static BOOL enumerate_parameters(FILE *fileIn, PM_PARMFUNC pfunc)
{
   char szBuf[PM_MAXLINE + 1];
   char *pszTemp;
   BOOL bRetval;
   long lFileOffset;
   int  cTemp;
   BOOL bParmFound;

   bRetval = False;
   bParmFound = False;
   while (True)
   {
      /* first remember where we are */
      if ((lFileOffset = ftell(fileIn)) >= 0L)
      {
	 /* then get and check a line */
	 if (fgets(szBuf, PM_MAXLINE, fileIn) == NULL)
	 {
	    /* stop - return OK unless file error */
	    bRetval = !ferror(fileIn);
            if (bRetval)
               Debug(3, "End of file encountered (enumerating parameters)!\n");
            else
               Debug(0, "Read error on configuration file (enumerating parameters)!\n");
	    break;   
	 }
	 else
	    /* if first non-white is a '[', stop (new section) */
	    if ((cTemp = firstnonwhite(szBuf)) == '[')
	    {
	       /* restore position to start of new section */
	       if (fseek(fileIn, lFileOffset, SEEK_SET) < 0L)
	       {
                  Debug(0, "Seek error on configuration file!\n");
                  break;
	       }

	       /* return success */
	       bRetval = True;
	       break;
	    }
	    else
	       /* if it's a semicolon or line is blank, ignore the line */
	       if (cTemp == ';' || cTemp == '\0')
	       {
		  continue;
	       }
	       else
		  /* if no equals sign and line contains non-whitespace */
		  /* then line is badly formed */
		  if ((pszTemp = strchr(szBuf, '=')) == NULL)
		  {
		     Debug(0, "Ignoring badly formed line: %s", szBuf);
		  }
		  else
		  {
                     /* Note that we have found a parameter */
                     bParmFound = True;
		     /* cut line at the equals sign */
		     *pszTemp++ = '\0';
		     /* trim leading and trailing space from both halves */
		     trimright(szBuf);
		     trimleft(szBuf);
		     trimright(pszTemp);
		     trimleft(pszTemp);
		     /* process the parameter iff passed pointer not NULL */
		     if (pfunc != NULL)
                        if (!pfunc(szBuf, pszTemp))
			   break;
		  }
      }
   }
   if (bRetval)
      if (!bParmFound)
         Debug(0, "Section %s contains no parameters (!?)\n");
   return (bRetval);
}

/**************************************************************************
Identifies all sections in the parameter file, calls passed section_func()
for each, passing the section name, then calls enumerate_parameters(). 
Returns True on success, False on failure. Note that the section and 
parameter names will have all internal whitespace areas collapsed to a 
single space for processing.
**************************************************************************/
static BOOL enumerate_sections(FILE *fileIn, 
			       PM_SECFUNC sfunc, PM_PARMFUNC pfunc)
{
   char szBuf[PM_MAXLINE + 1];
   BOOL bRetval;
   BOOL bSectionFound;

   bRetval = False;
   bSectionFound = False;
   while (True)
   {
      if (fgets(szBuf, PM_MAXLINE, fileIn) == NULL)
      {
	 /* stop - return OK unless file error */
	 bRetval = !ferror(fileIn);
         if (bRetval)
            Debug(3, "End of file encountered (enumerating sections)!\n");
         else
            Debug(0, "Read error on configuration file (enumerating sections)!\n");
	 break;   
      }
      else
      {
	 trimleft(szBuf);
	 trimright(szBuf);
	 if (szBuf[0] == '[')
	 {
	    closestr(szBuf, 0, 1);
	    if (strlen(szBuf) > 1)
	       if (szBuf[strlen(szBuf) - 1] == ']')
	       {  
		  /* found a section - note the fact */
                  bSectionFound = True;
		  /* remove trailing metabracket */
		  szBuf[strlen(szBuf) - 1] = '\0';
		  /* remove leading and trailing whitespace from name */
		  trimleft(szBuf);
		  trimright(szBuf);
		  /* reduce all internal whitespace to one space */
		  collapse_spaces(szBuf);
		  /* process it - stop if the processing fails */
		  if (sfunc != NULL)
                     if (!sfunc(szBuf))
		        break;
		  if (!enumerate_parameters(fileIn, pfunc))
                     break;
	       }
	 }
      }
   }
   if (bRetval)
      if (!bSectionFound)
         Debug(0, "Configuration file contains no sections (!?)\n");
   return (bRetval);
}

/**************************************************************************
Process the passed parameter file.

Returns True if successful, else False.
**************************************************************************/
BOOL pm_process(char *pszFileName, PM_SECFUNC sfunc, PM_PARMFUNC pfunc)
{
   FILE *fileIn;
   BOOL bRetval;

   bRetval = False;

   /* record the filename for use in error messages one day... */
   pszParmFile = pszFileName;

   if (pszParmFile == NULL || strlen(pszParmFile) < 1)
      Debug(0, "No configuration filename specified!\n");
   else
      if ((fileIn = fopen(pszParmFile, "rt")) == NULL)
         Debug(0, "Unable to open configuration file \"%s\"!\n", pszParmFile);
      else
      {
         Debug(2, "Processing configuration file \"%s\"\n", pszParmFile);
	 bRetval = enumerate_sections(fileIn, sfunc, pfunc);
	 fclose(fileIn);
      }

   if (!bRetval)
     Debug(0,"pm_process retuned false\n");
   return (bRetval);
}

/**************************************************************************
Strip all whitespace from a string.
**************************************************************************/
/*static void stripwhite(char *psz)
{
   char *pszDest;

   if ((pszDest = psz) != NULL)
   {
      while (*psz != '\0')
      {
       if (!isspace(*psz))
          *pszDest++ = *psz;
       psz++;
      }
      *pszDest = '\0';
   }
} */

/**************************************************************************
Strip all leading whitespace from a string.
**************************************************************************/
static void trimleft(char *psz)
{
   char *pszDest;

   pszDest = psz;
   if (psz != NULL)
   {
      while (*psz != '\0' && isspace(*psz))
	 psz++;
      while (*psz != '\0')
	 *pszDest++ = *psz++;
      *pszDest = '\0';
   }
}

/**************************************************************************
Strip all trailing whitespace from a string.
**************************************************************************/
static void trimright(char *psz)
{
   char *pszTemp;

   if (psz != NULL && psz[0] != '\0')
   {
      pszTemp = psz + strlen(psz) - 1;
      while (isspace(*pszTemp))
	 *pszTemp-- = '\0';
   }
}

/***********************************************************************
Close up psz by nCount chars, at offset iStart.
***********************************************************************/
static void closestr(char *psz, int iStart, int nCount)
{
   char *pszDest;
   int  nLen;

   if (nCount > 0)
      if ((pszDest = psz) != NULL)
      {
	 nLen = strlen(psz);
	 if (iStart >= 0 && iStart < nLen - nCount)
	 {
	    psz += iStart + nCount;
	    pszDest += iStart;

	    while (*psz)
	       *pszDest++ = *psz++;
	    *pszDest = '\0';
	 }
      }
}

/***********************************************************************
Collapse each whitespace area in a string to a single space.
***********************************************************************/
static void collapse_spaces(char *psz)
{
   while (*psz)
      if (isspace(*psz))
      {
	 *psz++ = ' ';
	 trimleft(psz);
      }
      else
	 psz++;
}

/**************************************************************************
Return the value of the first non-white character in the specified string.
The terminating NUL counts as non-white for the purposes of this function.
Note - no check for a NULL string! What would we return?
**************************************************************************/
static int firstnonwhite(char *psz)
{
   while (isspace(*psz) && (*psz != '\0'))
      psz++;
   return (*psz);
}
