/* local definitions for file server */
#ifndef _LOCAL_H
#define _LOCAL_H

/* This defines the section name in the configuration file that will contain */
/* global parameters - that is, parameters relating to the whole server, not */
/* just services. This name is then reserved, and may not be used as a       */
/* a service name. It will default to "global" if not defined here.          */
#define GLOBAL_NAME "global"

/* This defines the section name in the configuration file that will
   refer to the special "homes" service */
#define HOMES_NAME "homes"

/* set these to define the limits of the server. NOTE These are on a
   per-client basis. Thus any one machine can't connect to more than
   MAX_CONNECTIONS services, but any number of machines may connect at
   one time. */
#define MAX_CONNECTIONS 25
#define MAX_OPEN_FILES 50

/* max number of directories open at once */
#define MAXDIR 20

/* we need a suitable unsigned 2 byte and 1 byte int  */
#define WORD unsigned short
#ifndef BYTE
#define BYTE unsigned char
#endif

#define WORDMAX 0xFFFF


/* the print command on the server, %s is replaced with the filename  */
/* note that the -r removes the file after printing - you'll run out  */
/* of disk pretty quickly if you don't. This command is only used as  */
/* the default - it can be overridden in the configuration file.      */
#define PRINT_COMMAND "lpr -r %s"


/* define this if you want to stop spoofing with .. and soft links */
#define REDUCE_PATHS

/* define this to replace the string functions with debugging ones */
/* #define STRING_DEBUG */


#endif
