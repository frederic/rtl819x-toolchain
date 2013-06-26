/* -- updatedd: tzo.c --
 *
 * Copyright (C) 2004, 2005 Philipp Benner
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

#include <dprintf.h>
#include <get_connection.h>
#include <libexception_handle.h>
#include <ret_codes.h>
#include <version.h>

#include "tzo.h"

static void
tzo_print_usage(char *pname, FILE *fp)
{
	(void)fprintf(fp,
		      "\nUsage: %s [...] %s -- [OPTION]... [USERNAME:PASSWORD] HOSTNAME\n\n",
		      pname, COLORED("tzo"));
	(void)fprintf(fp,
		      "For security reasons use the environment variable LOGIN instead of\n"
		      "passing the login information directly.\n\n"

		      "Options:\n"
		      "   -4    --ipv4 <address>        ip address version 4\n"
		      "         --help                  print help and exit\n"
		      "         --version               display version information and exit\n\n"
		      
		      "WARNING: This plugin has never been tested due to the fact that\n"
		      "tzo.com is not free! If you are a registered tzo user please help\n"
		      "to complete this plugin.\n\n"

		      "Report bugs to <"EMAIL">.\n\n");

	return;
}

static void
tzo_print_version(FILE *fp)
{

	(void)fprintf(fp,
		      "\n" PNAME " plugin for tzo.com version " VERSION ",\n"
		      "Copyright (C) 2005 Philipp Benner.\n"
		      HOMEPAGE "\n\n"

		      "This is free software, and you are welcome to redistribute it\n"
		      "under certain conditions; see the source for copying conditions.\n"
		      "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
		      "FOR A PARTICULAR PURPOSE.\n\n");

	return;

}

static void
tzo_ret_msg(int mode, const char *fmt, ...)
{

	va_list az;

	va_start(az, fmt);
	(void)vs_warn(ret_msg_buf, BUFSIZE, mode, fmt, az);
	va_end(az);

	return;

}

int
tzo_dyndns(int argc, char *argv[])
{

	struct arguments args;
	const char *ptr;
	int s, ret;

	(void)memset(&args, 0, sizeof(struct arguments));
	
	if(tzo_get_flags(&args, argc, argv) != RET_OK) {
		return RET_WRONG_USAGE;
	}

	s = get_connection(DYNDNSHOST, PORT, &ptr);
	if(s == -1) {
		tzo_ret_msg(HERR, "%s: %s", ptr, DYNDNSHOST);
		ret = RET_WARNING;
	} else {
		ret = tzo_update_dyndns(s, &args);
                if(ret == RET_OK) {
                        ret = tzo_check_server_msg(s, args.hostname);
                }
		(void)close(s);
	}

	return ret;

}

static int
tzo_get_flags(struct arguments *args, int argc, char *argv[])
{

	int c;

	for(;;) {

		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",		1, 0, '4' },
			{ "help",		0, 0, 'h' },
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
		case 'h':
			tzo_print_usage(argv[ARGV_PNAME], stdout);
			exit(EXIT_SUCCESS);
		case 'v':
			tzo_print_version(stdout);
			exit(EXIT_SUCCESS);
		}
	}

	switch(argc-optind) {
        default:
		tzo_ret_msg(NONE, "wrong usage");
		return RET_WRONG_USAGE;

        case 2:
		args->login = getenv("LOGIN");
		if(args->login == NULL) {
			tzo_ret_msg(NONE,
				"environment variable LOGIN is empty");
			return RET_WRONG_USAGE;
		}
		break;
        case 3:
		args->login = argv[ARGV_LOGIN];
	}
	args->hostname = argv[ARGV_HOSTNAME];

	return RET_OK;

}

#define BUFFREE(name) BUFSIZE - strlen(name)

static int
tzo_update_dyndns(int s, struct arguments *args)
{

	char *user, *pass;
	char msg[BUFSIZE], server_msg[BUFSIZE];
        int len = strlen(args->login), ret;
        char login[len];

        strcpy(login, args->login);
	user = strtok(login, ":");
	pass = strtok(NULL, "");

        (void)snprintf(msg, BUFSIZE, "R %s,%s,%s",
                       args->hostname,
                       user, pass);

        if(args->ipv4) {
                (void)strncat(msg, ",", BUFFREE(msg));
                (void)strncat(msg, args->ipv4, BUFFREE(msg));
        }

        (void)memset(server_msg, 0, BUFSIZE);
        if(read(s, server_msg, BUFSIZE-1) == -1) {
                tzo_ret_msg(PERR, "%s: read() failed",
                        args->hostname);
                ret = RET_ERROR;
        } else {
                print_debug("Server Message: %s", server_msg);
                if(strstr(server_msg, "TZO/Linksys Update Server")) {
                        print_debug("Message: %s\n", msg);
                        (void)dprintf(s, "%s\r\n", msg);
                        ret = RET_OK;
                } else {
                        tzo_ret_msg(NONE, "%s: invalid server",
                                args->hostname);
                        ret = RET_ERROR;
                }
        }

	return ret;

}

static int
tzo_check_server_msg(int s, const char *hostname)
{

	char server_msg[BUFSIZE];
        int ret;

        (void)memset(server_msg, 0, BUFSIZE);
        if((ret = read(s, server_msg, BUFSIZE-1)) != -1) {
                print_debug("Server Message: %s", server_msg);
                
                tzo_ret_msg(NONE, "%s: %s",
                        hostname, server_msg+3);
                if(strncmp(server_msg, "40", 2) == 0) {
                        ret = RET_OK;
                } else {
                        ret = RET_ERROR;
                }
        } else {
                tzo_ret_msg(PERR, "%s: read() failed",
                        hostname);
                ret = RET_ERROR;
        }

        return ret;

}
