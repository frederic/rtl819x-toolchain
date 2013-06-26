/* -- updatedd: noip.h --
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

#define DYNDNSHOST		"corede.net"
#define COREDE_ERROR		"<error>"
#define COREDE_MSG		"<message>"
#define COREDE_RESULTDATA		"<resultData>"
#define COREDE_ID		"<id>"
#define COREDE_PWD		"<pwd>"
#define COREDE_DOMAIN		"<domain>"
#define COREDE_HOST		"<host>"
#define COREDE_INFO_FILE	"/var/corede_ddns"
#define COREDE_REGERR_FILE 	"/var/corede_err"
#define PORT		80

#define BUFSIZE		2048

#define ARGV_PNAME	0
#define ARGV_REGISTER  (argc-5)
#define ARGV_MAILADDR (argc-4)
#define ARGV_MAC  (argc-3)
#define ARGV_LOGIN	(argc-2)
#define ARGV_HOSTNAME	(argc-1)

#define COLORED(x)	"\033[0;35;1m"x"\033[0m"

static char ret_msg_buf[BUFSIZE];

struct arguments {
	char *hostname;
	char *group;
	char *ipv4;
	char *login;
	char *mac;
	char *mailaddr;
	char registered;
};

static struct dyndns_return_codes {
	const int code;
	const char *message;
	const int  error;
} return_codes[] = {
	{ 0,	"no update needed",				0 },
	{ 1,	"successfully updated",				0 },
	{ 2,	"bad hostname",					1 },
	{ 3,	"bad password",					1 },
	{ 4,	"bad user",					1 },
	{ 6,	"account has been banned",			1 },
	{ 7,	"invalid ip",					1 },
	{ 8,	"host has been disabled",			1 },
	{ 9,	"invalid host (web redirect)",			1 },
	{ 10,	"bad group",					1 },
	{ 11,	"group has been updated",			0 },
	{ 12,	"no update needed",				0 },
	{ 13,	"this client software has been disabled",	1 },
	{ 0,	NULL,						0 }
};

static int get_flags(struct arguments *args, int argc, char *argv[]);
static int update_corede(int s, struct arguments *args);
static int check_server_msg(int s, char *hostname);
static int register_corede(int s, struct arguments *args);

inline void
stolower(char *str, char *buf, size_t size)
{

	int n;

	for(n = 0; n < size && str[n] != '\0'; n++) {
		buf[n] = tolower(str[n]);
	}
	buf[n] = '\0';

	return;

}

char *
noip_get_retmsg(void)
{
	return ret_msg_buf;
}

