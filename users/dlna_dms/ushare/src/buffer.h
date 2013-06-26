/* buffer.h - String buffer manipulation tools header.
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

#ifndef _STRING_BUFFER_H_
#define _STRING_BUFFER_H_

struct buffer_t {
  char *buf;
  size_t len;
  size_t capacity;
};

struct buffer_t *buffer_new (void)
    __attribute__ ((malloc));
void buffer_free (struct buffer_t *buffer);

void buffer_append (struct buffer_t *buffer, const char *str);
void buffer_appendf (struct buffer_t *buffer, const char *format, ...)
    __attribute__ ((format (printf , 2, 3)));

#endif /* _STRING_BUFFER_H_ */
