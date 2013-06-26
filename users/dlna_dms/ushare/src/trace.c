/*
 * trace.c : GeeXboX uShare log facility.
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
#include <stdarg.h>
#include <syslog.h>
#include <stdbool.h>

#include "../config.h"
#include "trace.h"
#include "ushare.h"

extern struct ushare_t *ut;

void
print_log (log_level level, const char *format, ...)
{
  va_list va;
  bool is_daemon = ut ? ut->daemon : false;
  bool is_verbose = ut ? ut->verbose : false;

  if (!format)
    return;

  if (!is_verbose && level >= ULOG_VERBOSE)
    return;

  va_start (va, format);
  if (is_daemon)
  {
    int flags = LOG_DAEMON;
    flags |= level == ULOG_ERROR ? LOG_ERR : LOG_NOTICE;
    vsyslog (flags, format, va);
  }
  else
  {
    FILE *output = level == ULOG_ERROR ? stderr : stdout;
    vfprintf (output, format, va);
  }
  va_end (va);
}

inline void
start_log (void)
{
  openlog (PACKAGE_NAME, LOG_PID, LOG_DAEMON);
}
