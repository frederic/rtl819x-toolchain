/*
 * trace.h : GeeXboX uShare log facility headers.
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

#ifndef _TRACE_H_
#define _TRACE_H_

typedef enum {
  ULOG_NORMAL = 1,
  ULOG_ERROR = 2,
  ULOG_VERBOSE = 3,
} log_level;

void print_log (log_level level, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));
inline void start_log (void);

/* log_info
 * Normal print, to replace printf
 */
#define log_info(s, str...) {          \
  print_log (ULOG_NORMAL, (s), ##str); \
  }

/* log_error
 * Error messages, output to stderr
 */
#define log_error(s, str...) {        \
  print_log (ULOG_ERROR, (s), ##str); \
  }

/* log_verbose
 * Output only in verbose mode
 */
#define log_verbose(s, str...) {        \
  print_log (ULOG_VERBOSE, (s), ##str); \
  }

#endif /* _TRACE_H_ */
