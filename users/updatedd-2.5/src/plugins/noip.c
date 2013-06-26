/* -- updatedd: noip.c --
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

#include <get_connection.h>
#include <libexception_handle.h>
#include <ret_codes.h>
#include <version.h>

#include "noip.h"

static void
print_usage(char *pname, FILE *fp)
{
	(void)fprintf(fp,
		      "\nUsage: %s [...] %s -- [OPTION]... [USERNAME:PASSWORD] HOSTNAME\n\n",
		      pname, COLORED("noip"));
	(void)fprintf(fp,
		      "For security reasons use the environment variable LOGIN instead of\n"
		      "passing the login information directly.\n\n"
		      "Options:\n"
		      "   -4    --ipv4 <address>        ip address version 4\n"
/*                       "   -g    --group                 use group instead of HOSTNAME\n" */
		      "         --help                  print help and exit\n"
		      "         --version               display version information and exit\n\n"
		      "Report bugs to <"EMAIL">.\n\n");

	return;
}

static void
print_version(FILE *fp)
{

	(void)fprintf(fp,
		      "\n" PNAME " plugin for no-ip.com version " VERSION ",\n"
		      "Copyright (C) 2005 Philipp Benner.\n"
		      HOMEPAGE "\n\n"

		      "This is free software, and you are welcome to redistribute it\n"
		      "under certain conditions; see the source for copying conditions.\n"
		      "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
		      "FOR A PARTICULAR PURPOSE.\n\n");

	return;

}

static void
ret_msg(int mode, const char *fmt, ...)
{

	va_list az;

	va_start(az, fmt);
	(void)vs_warn(ret_msg_buf, BUFSIZE, mode, fmt, az);
	va_end(az);

	return;

}

int
noip_dyndns(int argc, char *argv[])
{

	struct arguments args;
	const char *ptr;
	int s, ret;

	(void)memset(&args, 0, sizeof(struct arguments));
	
	if(get_flags(&args, argc, argv) != RET_OK) {
		return RET_WRONG_USAGE;
	}

	s = get_connection(DYNDNSHOST, PORT, &ptr);
	if(s == -1) {
		ret_msg(HERR, "%s: %s", ptr, DYNDNSHOST);
		ret = RET_WARNING;
	} else {
		ret = update_dyndns(s, &args);
		if(ret == RET_OK) {
                        if(args.hostname) {
                                ret = check_server_msg(s, args.hostname);
                        } else {
                                ret = check_server_msg(s, args.group);
                        }
		}
		(void)close(s);
	}

	return ret;

}

static int
get_flags(struct arguments *args, int argc, char *argv[])
{

	int c;
	char **ptr = &args->hostname;

	for(;;) {

		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",		1, 0, '4' },
			{ "help",		0, 0, 'h' },
/* 			{ "group",		0, 0, 'g' }, */
			{ "version",		0, 0, 'v' },
			{ NULL,			0, 0, 0   }
		};

		c = getopt_long(argc, argv, "4:",
				long_options, &option_index);

		if(c == -1) break;

		switch(c) {
		case '4':
			args->ipv4 = optarg;
			break;
/* 		case 'g': */
/* 			ptr = &args->group; */
/* 			break; */
		case 'h':
			print_usage(argv[ARGV_PNAME], stdout);
			exit(EXIT_SUCCESS);
		case 'v':
			print_version(stdout);
			exit(EXIT_SUCCESS);
		}
	}

	switch(argc-optind) {
        default:
		ret_msg(NONE, "wrong usage");
		return RET_WRONG_USAGE;

        case 2:
		args->login = getenv("LOGIN");
		if(args->login == NULL) {
			ret_msg(NONE,
				"environment variable LOGIN is empty");
			return RET_WRONG_USAGE;
		}
		break;
        case 3:
		args->login = argv[ARGV_LOGIN];
	}
	*ptr = argv[ARGV_HOSTNAME];

	return RET_OK;

}

#define BUFLEN		4096
#define BUFFREE(name)	BUFLEN - strlen(name)

static int
update_dyndns(int s, struct arguments *args)
{

	char message[BUFLEN];
	int i;
// david ---------------------------	
//	int len = strlen(args->login);
//        char login[len], *pass = NULL;
	char login[100], *pass = NULL;
	int len = strlen(args->login);
//-----------------------------------

	strcpy(login, args->login);
        
	for(i = 0; i < len; i++) {
		if(login[i] == ':') {
			login[i] = '\0';
                        pass = login+i+1;
			break;
		}
	}
	if(pass == NULL) {
		ret_msg(NONE, "password is missing");
		return RET_WRONG_USAGE;
	}

	(void)snprintf(message, BUFLEN,
		       "GET /update.php?username=%s&pass=%s",
		       login, pass );

	if(args->ipv4) {
		(void)strncat(message, "&ip=", BUFFREE(message));
		(void)strncat(message, args->ipv4, BUFFREE(message));
	}

	if(args->hostname) {
		(void)strncat(message, "&host=", BUFFREE(message));
		(void)strncat(message, args->hostname, BUFFREE(message));
/*	} else if(args->group) {                                       *
 *		(void)strncat(message, "&group=", BUFFREE(message));   *
 *		(void)strncat(message, args->group, BUFFREE(message)); */
	} else {
		ret_msg(NONE, "wrong usage");
		return RET_WRONG_USAGE;
	}

	{
		char buffer[1024];

		(void)snprintf(buffer, 1024,
                               " HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Connection: close\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNDNSHOST, PNAME, VERSION, HOMEPAGE);
		(void)strncat(message, buffer, BUFFREE(message));
	}
	print_debug("\n\nMessage:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
                    message);
	
	if(write(s, message, strlen(message)) == -1) {
		ret_msg(PERR, "write() failed");
		return RET_WARNING;
	}

	return RET_OK;

}

static int
check_server_msg(int s, char *hostname)
{

	int n, ret_code;
	char server_msg[BUFSIZE], *ptr;

	(void)memset(server_msg, 0, sizeof(server_msg));
	if(read(s, server_msg, sizeof(server_msg) - 1) < 0) {
		ret_msg(PERR, "read() failed");
		return RET_WARNING;
        }

	print_debug("\n\nServer message:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
		    server_msg);

	if(strstr(server_msg, "HTTP/1.1 200 OK") ||
           strstr(server_msg, "HTTP/1.0 200 OK") ) {

		ptr = strstr(server_msg, "status=");
		ret_code = atoi((ptr+7));
		for(n=0; return_codes[n].message != NULL; n++) {
			if(ret_code == return_codes[n].code) {
				ret_msg(NONE, "%s: %s", hostname, return_codes[n].message);
				if(return_codes[n].error == 1) {
					return RET_ERROR;
				} else {
					break;
				}
			}
		}
	} else {
		ret_msg(NONE, "no-ip.com: Internal Server Error");
		return RET_ERROR;
	}

	return RET_OK;
}
