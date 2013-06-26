/* -- updatedd: dprintf.h --
 *
 * Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
 *
 * This file is part of UpdateDD - http://updatedd.philipp-benner.de.
 *
 * UpdateDD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * UpdateDD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UpdateDD; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HAVE_DPRINTF
# define HAVE_DPRINTF 1

# include <stdarg.h>
# include <sys/types.h>
# ifdef HAVE_ERROR_H
#  include <error.h>
# endif
# include <libexception_handle.h>

# define MAXLEN	2048

static inline int
dprintf(int s, const char *fmt, ...)
{

	char *buffer;
	int n;
	va_list va;

	va_start(va, fmt);
	n = vsprintf(NULL, fmt, va);
	if(n > MAXLEN)
		std_err(NONE, "dprintf() failed: string is too long");
	if((buffer = (char *)malloc((n+1) * sizeof(char))) == NULL)
		std_err(PERR, "malloc() failed");
	(void)vsnprintf(buffer, n+1, fmt, va);
	*(buffer+n) = '\0';
	va_end(va);

	if(write(s, buffer, n) == -1)
		n = -1;
	free(buffer);

	return n;

}

#endif /* HAVE_DPRINTF */
