/*
 * util_iconv.h : GeeXboX uShare iconv string encoding utilities headers.
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

#ifndef _UTIL_ICONV_H_
#define _UTIL_ICONV_H_

void setup_iconv (void);
void finish_iconv (void);
char *iconv_convert (const char *inbuf)
    __attribute__ ((malloc, nonnull, format_arg (1)));

#define UTF8 "UTF-8"

#endif
