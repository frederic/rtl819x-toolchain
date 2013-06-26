/* -- updatedd: dyndns.c --
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

#include <base64encode.h>
#include <get_connection.h>
#include <libexception_handle.h>
#include <ret_codes.h>
#include <version.h>

#include "atnet.h"
#if 0
static void
dyndns_print_usage(char *pname, FILE *fp)
{
	(void)fprintf(fp,
		      "\nUsage: %s [...] %s -- [OPTION]... [USERNAME:PASSWORD] HOSTNAME\n\n",
		      pname, COLORED("dyndns"));
	(void)fprintf(fp,
		      "For security reasons use the environment variable LOGIN instead of\n"
		      "passing the login information directly.\n\n"
		      "Options:\n"
		      "   -4    --ipv4 <address>        ip address version 4\n"
		      "   -b    --with-backmx           enable backmx\n"
		      "   -m    --mx <host>             hostname of your mail exchange\n"
		      "   -o    --offline               set host to offline mode\n"
		      "   -s    --system <system>       specify the system\n"
		      "                                 (dyndns [default], statdns, custom)\n"
		      "   -w    --wildcard <on|off>     switch wildcard on or off\n"
		      "         --help                  print help and exit\n"
		      "         --version               display version information and exit\n\n"
		      "Report bugs to <"EMAIL">.\n\n");
        
	return;
}
#endif
static void
print_version(FILE *fp)
{
        
	(void)fprintf(fp,
		      "\n" PNAME " plugin for dyndns.org version " VERSION ",\n"
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
atnet(int argc, char *argv[])
{
        
	struct arguments args;
	int s, ret;
	const char *ptr;

	(void)memset(&args, 0, sizeof(struct arguments));
	
	if(get_flags(&args, argc, argv) != RET_OK) {
		return RET_WRONG_USAGE;
	}
        
	s = get_connection(DYNDNSHOST, PORT, &ptr);
	if(s == -1) {
		ret_msg(HERR, "%s: %s", ptr, DYNDNSHOST);
		ret = RET_WARNING;
	} else {
		ret = update_atnet(s, &args);
		if(ret == RET_OK) {
			ret = check_server_msg(s, args.hostname);
		}
		(void)close(s);
	}
       if(ret == RET_OK)
       {
       	system("echo 1 > /var/ddns_status");
       }
	else
	{
		system("echo 0 > /var/ddns_status");
	}
	return ret;

}

static void
stolower(char *str, char *buf, size_t size)
{

	int n;
        
	for(n = 0; n < size && str[n] != '\0'; n++) {
		buf[n] = tolower(str[n]);
	}
	buf[n] = '\0';

	return;
        
}

static int
get_flags(struct arguments *args, int argc, char *argv[])
{
        
	int c;
        
	for(;;) {
                
		int option_index = 0;
		static struct option long_options[] = {
			{ "ipv4",		1, 0, '4' },
			{ "help",		0, 0, 'h' },
			{ "mx",			1, 0, 'm' },
			{ "offline",		0, 0, 'o' },
			{ "system",		1, 0, 's' },
			{ "wildcard",		1, 0, 'w' },
			{ "with-backmx",	0, 0, 'b' },
			{ "version",		0, 0, 'v' },
			{ NULL,			0, 0, 0   }
		};
                
		c = getopt_long(argc, argv, "4:bm:os:w:",
				long_options, &option_index);
                
		if(c == -1) break;
                
		switch(c) {
		case '4':
			args->ipv4 = optarg;
			break;
		case 'v':
			print_version(stdout);
			exit(EXIT_SUCCESS);
			break;
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
	args->hostname = argv[ARGV_HOSTNAME];
        
	return RET_OK;
        
}

#define BUFLEN		4096
#define BUFFREE(name)	BUFLEN - strlen(name)

static int
update_atnet(const int s, struct arguments *args)
{

	char *b64user;
	char message[BUFLEN];
        
	if(strlen(args->login) > 128) {
		ret_msg(NONE, "username is too long");
		return RET_ERROR;
	}
	memset(message,0,sizeof(message));
	
	b64user = (char *)malloc((2 * strlen(args->login) + 1));
	if(b64user == NULL) {
		ret_msg(PERR, "malloc() failed");
		return RET_WARNING;
	}
	(void)memset(b64user, 0, 2 * strlen(args->login) + 1);
        
	base64encode(args->login, b64user);
        
	{
		char buffer[1024];
                
		(void)snprintf(buffer, 1024,
                               "GET /DynDNS/dyndns.cgi HTTP/1.0\r\n"
                               "Host: %s\r\n"
                               "Authorization: Basic %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Connection: close\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNDNSHOST, b64user, PNAME, VERSION, HOMEPAGE);
		(void)strncat(message, buffer, BUFLEN - 1 - strlen(message));
	}
	print_debug("\n\nMessage:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
                    message);
	
	if(write(s, message, strlen(message)) == -1) {
		ret_msg(PERR, "write() failed");
		return RET_WARNING;
	}
        
	free(b64user);
	return RET_OK;
        
}
static int
clear_atnet(const int s, struct arguments *args)
{

	char *b64user;
	char message[BUFLEN];
        
	if(strlen(args->login) > 128) {
		ret_msg(NONE, "username is too long");
		return RET_ERROR;
	}
	memset(message,0,sizeof(message));
	
	b64user = (char *)malloc((2 * strlen(args->login) + 1));
	if(b64user == NULL) {
		ret_msg(PERR, "malloc() failed");
		return RET_WARNING;
	}
	(void)memset(b64user, 0, 2 * strlen(args->login) + 1);
        
	base64encode(args->login, b64user);
        
	{
		char buffer[1024];
                
		(void)snprintf(buffer, 1024,
                               " GET /DynDNS/clear.cgi HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Authorization: Basic %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Connection: close\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNDNSHOST, b64user, PNAME, VERSION, HOMEPAGE);
		(void)strncat(message, buffer, BUFLEN - 1 - strlen(message));
	}
	print_debug("\n\nMessage:"
		    "\n--------------------------------------\n"
		    "%s--------------------------------------\n\n",
                    message);
	
	if(write(s, message, strlen(message)) == -1) {
		ret_msg(PERR, "write() failed");
		return RET_WARNING;
	}
        
	free(b64user);
	return RET_OK;
        
}
/*
        Save Result Information
        args:
                name   :        the user name
                padding:        the domain name
                addr   :        ip address
        return :
                void   :        no return value
*/
void
WriteResult(char *body)
{
        FILE *fp=NULL;
        fp=fopen(ATNETDDNS_RESULT_FILE,"w");
        if(NULL == fp)
        {
                return;
        }
        fwrite(body,BUFSIZE/2,1,fp);
        fclose(fp);
        return;
}
/*
        Skip HTTP Respone Header
 
        args:
                response:       HTTP response
        returns:
                body:           HTTP body message
*/
char *
SkipHeader( response )
char    *response;
{
        char    *cur;                   /* current position */
        char    *body;                  /* temporaly pointer */
 
        /* skip response header */
        for( cur = response ; cur != NULL ; cur++ ){
                cur = index( cur, '\n' );
 
                /* header/body separator is "\r\n\r\n" */
                if( *(cur-1) == '\r' && *(cur+1) == '\r' && *(cur+2) == '\n' ){
                        *(cur-1) = '\0';
                        body = cur + 3;
                        break;
                }
        }
 
        /* return body head */
        return( body );
}

static int
check_server_msg(int s, const char *hostname)
{
        
	int n;
	char server_msg[BUFSIZE], *ptr , *body;
        
	/* get server_msg */
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
	   strstr(server_msg, "HTTP/1.0 200 OK")) {
                body = SkipHeader(server_msg);
		WriteResult(body);
		return RET_OK;
#if 0
		(void)strtok(server_msg, "\n");
		while((ptr = strtok(NULL, "\n")) != NULL) {
			for(n=0; return_codes[n].code != NULL; n++) {
				if(strstr(ptr, return_codes[n].code)) {
					ret_msg(NONE, "%s: %s",
						hostname, return_codes[n].message);
					if(return_codes[n].error == 1) {
						return RET_ERROR;
					} else {
						return RET_OK;
					}
				}
			}
		}
#endif
	} else if(strstr(server_msg, "401 Authorization Required")) {
		ret_msg(NONE, "dyndns.org: wrong username or password");
	} else {
		ret_msg(NONE, "dyndns.org: Internal Server Error");
	}
        
	return RET_ERROR;
}

