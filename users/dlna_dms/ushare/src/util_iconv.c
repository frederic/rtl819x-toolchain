/*
 * util_iconv.c : GeeXboX uShare iconv string encoding utlities.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Alexis Saettler <asbin@asbin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util_iconv.h"

#if HAVE_ICONV
#include <iconv.h>
static iconv_t cd = 0;
#endif

#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

void
setup_iconv (void)
{
#if HAVE_ICONV && HAVE_LANGINFO_CODESET
  char *mycodeset = NULL;

  mycodeset = nl_langinfo (CODESET);
  if (!mycodeset)
    return;

  /**
   * Setup conversion descriptor if user's console is non-UTF-8. Otherwise
   * we can just leave cd as NULL
   */
  if (strcmp (mycodeset, UTF8))
  {
    cd = iconv_open (UTF8, mycodeset);
    if (cd == (iconv_t) (-1))
    {
      perror ("iconv_open");
      cd = 0;
    }
  }
#endif
}

void
finish_iconv (void)
{
#if HAVE_ICONV
  if (!cd)
    return;
  if (iconv_close (cd) < 0)
    perror ("iconv_close");
  cd = 0;
#endif
}

/**
 * iconv_convert : convert a string, using the current codeset
 * return: a malloc'd string with the converted result
 */
char *
iconv_convert (const char *input)
{
#if HAVE_ICONV
  size_t inputsize = strlen (input) + 1;
  size_t dummy = 0;
  size_t length = 0;
  char *result;
  char *inptr, *outptr;
  size_t insize, outsize;

  /* conversion not necessary. save our time. */
  if (!cd)
    return strdup (input);

  /* Determine the length we need. */
  iconv (cd, NULL, NULL, NULL, &dummy);
  {
    static char tmpbuf[BUFSIZ];
    inptr = (char*) input;
    insize = inputsize;
    while (insize > 0)
    {
      outptr = tmpbuf;
      outsize = BUFSIZ;
      if (iconv (cd, &inptr, &insize, &outptr, &outsize) == (size_t) (-1))
      {
        /**
         * if error is EINVAL or EILSEQ, conversion must be stoped,
         * but if it is E2BIG (not enough space in buffer), we just loop again
         */
        if( errno != E2BIG)
        {
          perror ("error iconv");
          return NULL;
        }
      }
      length += outptr - tmpbuf;
    }

    outptr = tmpbuf;
    outsize = BUFSIZ;
    if (iconv (cd, NULL, NULL, &outptr, &outsize) == (size_t) (-1))
    {
      perror ("error iconv");
      return NULL;
    }
    length += outptr - tmpbuf;
  }

  /* length determined, allocate result space */
  if ((result = (char*) malloc (length * sizeof (char))) == NULL)
  {
    perror ("error malloc");
    return NULL;
  }

  /* Do the conversion for real. */
  iconv (cd, NULL, NULL, NULL, &dummy);
  {
    inptr = (char*) input;
    insize = inputsize;
    outptr = result;
    outsize = length;
    while (insize > 0)
    {
      if (iconv (cd, &inptr, &insize, &outptr, &outsize) == (size_t) (-1))
      {
        if (errno != E2BIG)
        {
          perror ("error iconv");
          free (result);
          return NULL;
        }
      }
    }
    if (iconv (cd, NULL, NULL, &outptr, &outsize) == (size_t) (-1))
    {
      perror ("error iconv");
      free (result);
      return NULL;
    }

    if (outsize != 0)
      abort ();
  }

  return result;
#else
  return strdup (input);
#endif
}
