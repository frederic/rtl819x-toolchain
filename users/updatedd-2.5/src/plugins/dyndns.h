/* -- updatedd: dyndns.h --
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

#include <ret_codes.h>

#define DYNDNSHOST	"members.dyndns.org"
#define PORT		80

#define BUFSIZE		512

#define ARGV_PNAME	0
#define ARGV_LOGIN	argc-2
#define ARGV_HOSTNAME	argc-1

#define COLORED(x)	"\033[0;31;1m"x"\033[0m"

#define DYNDNS_RESULT_FILE "/var/tmp/ddns_result.html"
static char ret_msg_buf[BUFSIZE];

struct arguments {
	const char *backmx;
	const char *hostname;
	char *ipv4;
	char *mx;
	const char *offline;
	const char *system;
	char *login;
	const char *wildcard;
};

static struct yesno {
	const char *yes;
	const char *no;
} yn = { "yes", "no" };

static const char *dd_system[] = { 
	"dyndns",
	"statdns",
	"custom",
	NULL
};

static struct dyndns_return_codes {
	const char *code;
	const char *message;
	const int  error;
} return_codes[] = {
	{ "badauth",	"Bad authorization (username or password).",		1 },
	{ "badsys",	"The system parameter given was not valid.",		1 },
	{ "badagent",	"The useragent your client sent has been blocked "
          "at the access level.",						1
	},
	{ "good",	"Update good and successful, IP updated.",		0 },
	{ "nochg",	"No changes, update considered abusive.",		0 },
	{ "notfqdn",	"A Fully-Qualified Domain Name was not provided.",	1 },
	{ "nohost",	"The hostname specified does not exist.",		1 },
	{ "!donator",	"The offline setting was set, when the user is "
          "not a donator.",							1
	},
	{ "!yours",	"The hostname specified exists, but not under "
          "the username currently being used.",					1
	},
	{ "!active",	"The hostname specified is in a Custom DNS "
          "domain which has not yet been activated.",				1
	},
	{ "abuse",	"The hostname specified is blocked for abuse",		1 },
	{ "notfqdn",	"No hosts are given.",					1 },
	{ "numhost",	"Too many or too few hosts found.",			1 },
	{ "dnserr",	"DNS error encountered.",				1 },
	{ NULL,		NULL,							0 }
};

static int get_flags(struct arguments *args, int argc, char *argv[]);
static int update_dyndns(const int s, struct arguments *args);
static int check_server_msg(const int s, const char *hostnames);

char *
get_retmsg(void)
{
	return ret_msg_buf;
}
