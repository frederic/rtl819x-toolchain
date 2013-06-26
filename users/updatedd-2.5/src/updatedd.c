/* -- updatedd: updatedd.c --
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
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#define SYSLOG
#include <libexception_handle.h>
#include <ret_codes.h>
#include <unistd.h>
#include <getopt.h>
#include <version.h>
#include "updatedd.h"

#ifdef __DARWIN__
#  define PLUGIN_ENDING ".bundle"
#else
#  define PLUGIN_ENDING ".so"
#endif

void
print_usage(char *pname, FILE *fp)
{

	(void)fprintf(fp,
                      "\nUsage: %s [OPTION]... SERVICE -- ...\n\n", pname);
	(void)fprintf(fp,
		      "Please use `updatedd-wrapper' instead of updatedd.\n\n"
                      "Options:\n"
                      "   -L		list installed plugins (services) and exit\n"
                      "   -Y		use syslog\n"
                      "   --help	print help and exit\n"
                      "   --version	print version information and exit\n\n"
                      "Report bugs to <"EMAIL">.\n\n");

	return;

}

void
print_version(FILE *fp)
{

	(void)fprintf(fp,
                      "\n" PNAME " version " VERSION ", Copyright (C) 2005 Philipp Benner.\n"
                      HOMEPAGE "\n\n"

                      "This is free software, and you are welcome to redistribute it\n"
                      "under certain conditions; see the source for copying conditions.\n"
                      "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
                      "FOR A PARTICULAR PURPOSE.\n\n");

	return;

}

void
wrong_usage(const char *msg)
{

	if(msg != NULL) {
		(void)fprintf(stderr, "%s\n", msg);
	}
	(void)fprintf(stderr,
		      "Try `updatedd --help' for more information.\n");

	exit(EXIT_FAILURE);

}

void *
get_libfunc(char *lib, const char *lib_func)
{

	const char *c_error;
	void *handle;
	void *function;
	char path[256];

	(void)memset(path, 0, sizeof(path));
	(void)snprintf(path, sizeof(path)-1, "%s/%s" PLUGIN_ENDING, LIBPATH, lib);

	handle = dlopen(path, RTLD_LAZY);
	if(!handle) {
		err(NONE, "%s", dlerror());
	}

	function = dlsym(handle, lib_func+1);
	if((c_error = dlerror()) != NULL) {
		function = dlsym(handle, lib_func);
		if((c_error = dlerror()) != NULL) {
			err(NONE, "%s: %s", c_error, path);
		}
	}

	return function;

}

int
get_service(DIR *dir, char *buf, size_t size)
{

	struct dirent *dir_info;

	while( (dir_info = readdir(dir)) ) {
		int n;
		char *ptr = strstr(dir_info->d_name, PLUGIN_ENDING);
		if(ptr != NULL) {
			for(n = 0; dir_info->d_name+n != ptr; n++) {
				buf[n] = dir_info->d_name[n];
			}
			buf[n] = '\0';
			return 1;
		}
	}

	return 0;

}

int
update_ddns(char *service, int argc, char *argv[])
{

	int ret;
	lib_main	ptr_main;
	lib_getretmsg	ptr_getretmsg;

	ptr_main	= (lib_main)get_libfunc(service, LIB_MAIN);
	ptr_getretmsg	= (lib_getretmsg)get_libfunc(service, LIB_GETRETMSG);

	ret = (*ptr_main)(argc, argv);

	switch(ret) {
        case RET_WRONG_USAGE:
		warn(NONE, "%s", (*ptr_getretmsg)());
		(void)fprintf(stderr,
                              "Try `updatedd %s -- --help' for more information.\n",
                              service);
		break;
        case RET_WARNING:
        case RET_ERROR:
		warn(NONE, "%s", (*ptr_getretmsg)());
		break;
        case RET_OK:
		notice(NONE, "%s", (*ptr_getretmsg)());

	}

	return ret;

}
extern int dyndns(int argc, char *argv[]);
extern int tzo_dyndns(int argc, char *argv[]);
#ifdef CONFIG_RTL865X_GRC
extern int corede(int argc, char *argv[]);
extern int atnet(int argc, char *argv[]);
#endif

//extern int noip_dyndns(int argc, char *argv[]);
int
exec_plugin(char *service, int argc, char *argv[])
{
	/*

	DIR *dir;
	char buf[BUFSIZE];
	int ret;

	dir = opendir(LIBPATH);
	if(dir == NULL) {
		err(PERR, "opendir() failed: %s", LIBPATH);
	}
	*/
	int ret;

        if(strcmp(service,"dyndns") == 0)
	         ret=dyndns(argc,argv);
        if(strcmp(service,"tzo") == 0)
	          ret=tzo_dyndns(argc,argv);
#ifdef CONFIG_RTL865X_GRC
	 if(strcmp(service,"corede") == 0)
	 	   ret=corede(argc,argv);
	 if(strcmp(service,"atnet") == 0)
	 	   ret=atnet(argc,argv);
#endif

#if 0
        if(strcmp(service,"noip") == 0)
	          noip_dyndns(argc,argv);
#endif
        return ret;
				
	

	/* check if the plugin really exist */
	/*
	while(get_service(dir, buf, BUFSIZE)) {
		if(strcmp(buf, service) == 0) {
			ret = update_ddns(service, argc, argv);
			(void)closedir(dir);
			return ret;
		}
	}
	(void)closedir(dir);

	warn(NONE, "no such plugin: %s", service);
	*/
//	return RET_ERROR;

}

void
list_services(void)
{

	DIR *dir;
	char buf[BUFSIZE];

	dir = opendir(LIBPATH);
	if(dir == NULL) {
		err(PERR, "opendir() failed: %s", LIBPATH);
	}

	(void)printf("\nServices:\n");
	while(get_service(dir, buf, BUFSIZE)) {
		(void)printf("%s\n", buf);
	}
	(void)printf("\n");

	(void)closedir(dir);

	return;

}

int
main(int argc, char *argv[])
{

	char *service;

	if(argc == 1) {
		wrong_usage("too few arguments");
		exit(RET_WRONG_USAGE);
	}

	for(;;) {
		int c, option_index = 0;
		static struct option long_options[] = {
			{ "help",	0, 0, 'h' },
			{ "version",	0, 0, 'v' }
		};

		c = getopt_long(argc, argv, "LY",
				long_options, &option_index);

		if(c == -1) {
			break;
		}

		switch(c) {
                case 'L':
			list_services();
			exit(RET_OK);
                case 'Y':
			use_syslog = 1;
			break;
                case 'h':
			print_usage(argv[0], stdout);
			exit(RET_OK);
                case 'v':
			print_version(stdout);
			exit(RET_OK);
		default:
			wrong_usage(NULL);
			exit(RET_WRONG_USAGE);
		}
	}

	service = argv[optind];

	return exec_plugin(service, argc, argv);

}
