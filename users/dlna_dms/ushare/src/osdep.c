/*
 * osdep.c : GeeXboX uShare OS independant helpers.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
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

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "osdep.h"

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
char *
strndup (const char *s, size_t n)
{
  size_t len;
  char *sdup = NULL;

  if (!s)
    return NULL;

  len = strlen (s);
  len = n < len ? n : len;
  sdup = (char *) malloc (len + 1);

  if (sdup)
  {
    memcpy (sdup, s, len);
    sdup[len] = '\0';
  }

  return sdup;
}

ssize_t
getline (char **lineptr, size_t *n, FILE *stream)
{
  static char line[256];
  char *ptr;
  ssize_t len;

  if (!lineptr || !n)
    return -1;

  if (ferror (stream))
    return -1;

  if (feof (stream))
    return -1;

  fgets (line, 256, stream);
  ptr = strchr (line, '\n');

  if (ptr)
    *ptr = '\0';

  len = strlen (line);
  if ((len + 1) < 256)
  {
    ptr = realloc (*lineptr, 256);
    if (!ptr)
      return -1;

    *lineptr = ptr;
    *n = 256;
  }
  strcpy (*lineptr, line);

  return len;
}
#endif /* (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__)) */
