/* buffer.c - String buffer manipulation tools.
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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "buffer.h"
#include "minmax.h"

#define BUFFER_DEFAULT_CAPACITY 32768

struct buffer_t *
buffer_new (void)
{
  struct buffer_t *buffer = NULL;

  buffer = (struct buffer_t *) malloc (sizeof (struct buffer_t));
  if (!buffer)
    return NULL;

  buffer->buf = NULL;
  buffer->len = 0;
  buffer->capacity = 0;

  return buffer;
}

void
buffer_append (struct buffer_t *buffer, const char *str)
{
  size_t len;

  if (!buffer || !str)
    return;

  if (!buffer->buf)
  {
    buffer->capacity = BUFFER_DEFAULT_CAPACITY;
    buffer->buf = (char *) malloc (buffer->capacity * sizeof (char));
    memset (buffer->buf, '\0', buffer->capacity);
  }

  len = buffer->len + strlen (str);
  if (len >= buffer->capacity)
  {
    buffer->capacity = MAX (len + 1, 2 * buffer->capacity);
    buffer->buf = realloc (buffer->buf, buffer->capacity);
  }

  strcat (buffer->buf, str);
  buffer->len += strlen (str);
}

void
buffer_appendf (struct buffer_t *buffer, const char *format, ...)
{
  char str[BUFFER_DEFAULT_CAPACITY];
  int size;
  va_list va;

  if (!buffer || !format)
    return;

  va_start (va, format);
  size = vsnprintf (str, BUFFER_DEFAULT_CAPACITY, format, va);
  if (size >= BUFFER_DEFAULT_CAPACITY)
  {
    char* dynstr = (char *) malloc (size + 1);
    vsnprintf (dynstr, size + 1, format, va);
    buffer_append (buffer, dynstr);
    free (dynstr);
  }
  else
    buffer_append (buffer, str);
  va_end (va);
}

void
buffer_free (struct buffer_t *buffer)
{
  if (!buffer)
    return;

  if (buffer->buf)
    free (buffer->buf);
  free (buffer);
}
