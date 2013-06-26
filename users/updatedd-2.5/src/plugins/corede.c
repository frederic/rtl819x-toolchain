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
#include <base64encode.h>

#include "corede.h"

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
corede(int argc, char *argv[])
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
	} 
	else if(args.registered)
	{
		//register
		ret=register_corede(s, &args);
		if(ret == RET_OK) {
                        if(args.hostname) {			
                                ret = check_server_msg(s, args.hostname);
                        } else {
                                ret = check_server_msg(s, args.group);
                        }
		}
	}
	else
	{
		ret = update_corede(s, &args);
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
			{ "register",	1, 0, 'r' },
			{ "hwaddr",	1, 0, 'a' },
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
		case 'r':
			args->registered=1;
			break;
		case 'a':
			args->mac = optarg;
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
		break;
	 case 4:
	 	args->login = argv[ARGV_LOGIN];
		args->mac=argv[ARGV_MAC];
		break;
	 case 6: /*add two more parameter*/
		args->login = argv[ARGV_LOGIN];
		args->registered=atoi(argv[ARGV_REGISTER]);
		args->mailaddr=argv[ARGV_MAILADDR];
		args->mac=argv[ARGV_MAC];
		break;
	}
	*ptr = argv[ARGV_HOSTNAME];

	return RET_OK;

}

#define BUFLEN		4096
#define BUFFREE(name)	BUFLEN - strlen(name)

static int register_corede(int s, struct arguments *args)
{
	char message[BUFLEN];
	char content[128];
	if(!args->mailaddr || !args->mac)
	{
		ret_msg(NONE, "wrong usage");
		return RET_WRONG_USAGE;
	}
	
	(void)snprintf(content, BUFLEN,
		       "mail=%s&mac=%s",
		       args->mailaddr, args->mac);

	{
		char buffer[1024];

		(void)snprintf(buffer, 1024,
                               "POST /register.php HTTP/1.0\r\n"
                               "Host: %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Content-Length: %d\r\n"
                               "Connection: close\r\n"
                               "Content-Type:application/x-www-form-urlencoded\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNDNSHOST, PNAME, VERSION, HOMEPAGE,strlen(content));
		(void)strncat(message, buffer, BUFFREE(message));
		(void)strncat(message,content,BUFFREE(message));
		(void)strncat(message,"\r\n",BUFFREE(message));
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
update_corede(int s, struct arguments *args)
{

	char message[BUFLEN];
	char content[256];
	char loginencode[64];
	char passencode[80];
	char macencode[32];
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
	memset(loginencode,0,sizeof(loginencode));
	memset(passencode,0,sizeof(passencode));
	memset(macencode,0,sizeof(macencode));
	
	base64encode(login,loginencode);
	base64encode(pass,passencode);
	base64encode(args->mac,macencode);
	
	(void)snprintf(content, 128,
		       "id=%s&pwd=%s&mac=%s",
		      loginencode, passencode,macencode);

	{
		char buffer[1024];

		(void)snprintf(buffer, 1024,
                               "POST /update.php HTTP/1.0\r\n"
                               "Host: %s\r\n"
                               "User-Agent: %s %s - %s\r\n"
                               "Content-Length: %d\r\n"
                               "Connection: close\r\n"
                               "Content-Type:application/x-www-form-urlencoded\r\n"
                               "Pragma: no-cache\r\n\r\n",
                               DYNDNSHOST, PNAME, VERSION, HOMEPAGE,strlen(content));
		(void)strncat(message, buffer, BUFFREE(message));
		(void)strncat(message,content,BUFFREE(message));
		(void)strncat(message,"\r\n",BUFFREE(message));
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

	int ret_code;
	char server_msg[BUFSIZE], *ptr;
	char  id[64],pwd[128],domain[128],host[64],tmpstr[128],retcodestr[6],*tmpptr;
	FILE *fp;

	(void)memset(server_msg, 0, sizeof(server_msg));
	(void)memset(id, 0, sizeof(id));
	(void)memset(pwd, 0, sizeof(pwd));
	(void)memset(domain, 0, sizeof(domain));
	(void)memset(host, 0, sizeof(host));
	(void)memset(tmpstr, 0, sizeof(tmpstr));
	(void)memset(retcodestr, 0, sizeof(retcodestr));

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
		/*move to the Data*/
		ptr = strstr(server_msg, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
		ptr = strstr(ptr,COREDE_ERROR);
		tmpptr = strstr(ptr,"</error>");
		ptr+=strlen(COREDE_ERROR);
		strncpy(retcodestr,ptr,tmpptr-ptr);
		ret_code=atoi(retcodestr);
		if(ret_code)
		{
			/*Error msg*/
			ptr=strstr(ptr,COREDE_MSG);
			tmpptr=strstr(ptr,"</message>");
			ptr +=strlen(COREDE_MSG);
			strncpy(server_msg,ptr,tmpptr-ptr);

		}
		else
		{
			ptr = strstr(ptr,COREDE_RESULTDATA);
			if (ptr)
			{
				/*register success*/
				ptr = strstr(ptr,COREDE_ID);
				tmpptr = strstr(ptr,"</id>");
				ptr+=strlen(COREDE_ID);
				strncpy(id,ptr,tmpptr-ptr);

				ptr = strstr(ptr,COREDE_PWD);
				tmpptr = strstr(ptr,"</pwd>");
				ptr+=strlen(COREDE_PWD);
				strncpy(pwd,ptr,tmpptr-ptr);

				ptr = strstr(ptr,COREDE_DOMAIN);
				tmpptr = strstr(ptr,"</domain>");
				ptr+=strlen(COREDE_DOMAIN);
				strncpy(domain,ptr,tmpptr-ptr);

				ptr = strstr(ptr,COREDE_HOST);
				tmpptr = strstr(ptr,"</host>");
				ptr+=strlen(COREDE_HOST);
				strncpy(host,ptr,tmpptr-ptr);

				/*Write to File*/
				fp=fopen(COREDE_INFO_FILE,"w");
				if(!fp)
				{
					return RET_ERROR;
				}
				base64decode(tmpstr,id,sizeof(tmpstr));
				fprintf(fp,"id:%s\n",tmpstr);
				(void)memset(tmpstr, 0, sizeof(tmpstr));
				base64decode(tmpstr,pwd,sizeof(tmpstr));
				fprintf(fp,"pwd:%s\n",tmpstr);
				fprintf(fp,"domain:%s\n",domain);
				fprintf(fp,"host:%s\n",host);
				fclose(fp);
			}
			else
			{
				/*update success*/
			}
		}
		fp=fopen(COREDE_REGERR_FILE,"w");
		if(!fp)
		{
			return RET_ERROR;
		}
		fprintf(fp,"%d\n",ret_code);
//		fprintf(fp,"%s\n",server_msg);
		fclose(fp);
	} else {
		ret_msg(NONE, "corede.net: Internal Server Error");
		return RET_ERROR;
	}
	return RET_OK;
}

