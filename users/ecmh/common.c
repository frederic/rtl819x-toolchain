/*****************************************************
 ecmh - Easy Cast du Multi Hub - Common Functions
******************************************************
 $Author: fuzzel $
 $Id: common.c,v 1.1.1.1 2004/01/10 23:59:32 fuzzel Exp $
 $Date: 2004/01/10 23:59:32 $
*****************************************************/

#include "ecmh.h"

void dolog(int level, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (g_conf->daemonize) vsyslog(LOG_LOCAL7|level, fmt, ap);
	else vprintf(fmt, ap);
	va_end(ap);
}

int huprunning()
{
	int pid;

	FILE *f = fopen(PIDFILE, "r");
	if (!f) return 0;
	fscanf(f, "%d", &pid);
	fclose(f);
	// If we can HUP it, it still runs
	return (kill(pid, SIGHUP) == 0 ? 1 : 0);
}

void savepid()
{
	FILE *f = fopen(PIDFILE, "w");
	if (!f) return;
	fprintf(f, "%d", getpid());
	fclose(f);

	dolog(LOG_INFO, "Running as PID %d\n", getpid());
}

void cleanpid(int i)
{
	dolog(LOG_INFO, "Exiting...\n");
	unlink(PIDFILE);
	exit(0);
}
