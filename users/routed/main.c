/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char copyright[] =
  "@(#) Copyright (c) 1983, 1988, 1993\n"
  "      The Regents of the University of California.  All rights reserved.\n";

/*
 * From: @(#)main.c	5.23 (Berkeley) 7/1/91
 * From: @(#)main.c	8.1 (Berkeley) 6/5/93
 */
char main_rcsid[] = 
  "$Id: main.c,v 1.1 2009/08/24 10:25:42 bradhuang Exp $";

#include "version.h"
#include "built_time"
#define VERSION_STR	"v1.0"

#define DISPLAY_BANNER \
	printf("\nRIP Routed %s (%s).\n\n", VERSION_STR, BUILT_TIME)
/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#include <sys/ioctl.h>
#include <sys/file.h>

#include <errno.h>
/* #include <signal.h>  (redundant with defs.h) */
#include <syslog.h>
#include <assert.h>
#include <sys/utsname.h>
#if 0//WiSOC unused
//ql
#include <config/autoconf.h>
#endif
#define BUFSPACE (127*1024)	/* max. input buffer size to request */

struct sockaddr_in addr;	/* address of daemon's socket */
int sock;			/* source and sink of all data */
char	packet[MAXPACKETSIZE+1];
int rip_port;

int	supplier = -1;		/* process should supply updates */
int	gateway = 1;		/* 1 if we are a gateway to parts beyond */
int	debug = 0;
int ripversion = 2;
int multicast = 1;
struct	rip *msg = (struct rip *)packet;
int	kernel_version;

static void getkversion(void);
static int getsocket(void);
static void process(int);

int rip_request_send(void)
{
	struct rip *query = msg;
	query->rip_cmd = RIPCMD_REQUEST;
	query->rip_vers = ripversion;
	memset(query->rip_nets, 0, sizeof(query->rip_nets));
	query->rip_nets[0].n_family = htons((u_short)AF_UNSPEC);
	query->rip_nets[0].n_metric = htonl((u_long)HOPCNT_INFINITY);
	toall(sndmsg, 0, NULL);

}

char* runPath = "/bin/routed";
char* cfgFile = "/var/run/routed.conf";
char* pidFile = "/var/run/routed.pid";

void
write_pid()
{
	FILE *fp = fopen(pidFile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}

void
clear_pid()
{
	FILE *fp = fopen(pidFile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", 0);
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}

int check_pid()
{
	FILE *fp = fopen(pidFile, "r");
	if (fp) {
		fclose(fp);
		return 0;	// pid exist
	}
	else
	 	return -1;
}


#define  ASCII_CR    0x0d
#define  ASCII_LF    0x0a
#define	 ASCII_SPACE 0x20
static char ch_buf[256];
static int buf_idx = 0;
static int last_buf_idx = 0;
int kbd_proc(FILE *fp)
{
int kbd_cc;
	if(!fp)
		return 0;
		
    if((kbd_cc=fgetc(fp)) == EOF)
    	return -1;
    switch(kbd_cc) {
    case 0:
    	/* get nothing */
    	break;
	case ASCII_LF:
	case ASCII_CR:
		ch_buf[buf_idx]='\0';
		last_buf_idx = buf_idx;
		buf_idx = 0;
		return 1;
	default:
		ch_buf[buf_idx]=kbd_cc;
	    if(buf_idx<sizeof(ch_buf))
			buf_idx++;
    }
	return 0;
}



#define MAX_ARGS	20
#define MAX_ARG_LEN	20
int cfg_proc(void)
{
unsigned int	i;
char argv[MAX_ARGS][MAX_ARG_LEN+1];
int	argc = 0;
int arg_idx = 0;

	for(i=0; ch_buf[i]!='\0'; i++)
	{
		if(ch_buf[i]==' '){
			argv[argc][arg_idx]='\0';
			argc++;
			arg_idx=0;
		}
		else {
			if(arg_idx<MAX_ARG_LEN)
			{	
				argv[argc][arg_idx]=ch_buf[i];
				arg_idx++;
			}
		}
	}
	argv[argc][arg_idx]='\0';
	//printf("cfg file arg[0]=%s\n",argv[0]);
	if(!strcmp(argv[0], "version")) {
		int new_ripversion = atoi(argv[1]);
		if(ripversion!=new_ripversion) {
			if(ripversion && new_ripversion == 0){				
				clean();
			}
			if(ripversion==0 && new_ripversion) {
				rtinit();
				ifinit();
				rip_input_init();
				rip_request_send();
			}
			ripversion = new_ripversion;
		}		
	}
	else
	if(!strcmp(argv[0], "network")) {
		// Modified by Mason Yu for bind interface
		//if_chk_add(argv[1]);
#ifndef CONFIG_BOA_WEB_E8B_CH
		if_chk_add(argv[1], atoi(argv[2]), atoi(argv[3]));
#else
		if_chk_add(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
#endif
	}
	else
	if(!strcmp(argv[0], "debug")) {
		debug = 1;
	}

	memset(argv, 0, sizeof(argv));
	return 0;
}

int write_cfg(void)
{
	FILE *fp = fopen(cfgFile, "w+");
	if (fp) {
		fprintf(fp, "version %d\n", ripversion);
		if_cfg_write(fp);
		fclose(fp);
	}
	else
	 	printf("Cannot create cfg file\n");
	
}

int read_cfg(void)
{
	FILE *fp = fopen(cfgFile, "r");
	int got_line;
	if (fp) {
		debug = 0;
		while((got_line=kbd_proc(fp)) != -1) {
			if(got_line)
				cfg_proc();
		}
		if_cfg_refresh();
		fclose(fp);
		return 0;
	}
	else
	 	printf("Cannot read cfg file\n");
	return -1;
}




int
main(int argc, char *argv[])
{
	int _argc = 0;
	char *_argv[5];
	pid_t pid;
	int execed = 0;

	int n, nfd, tflags = 0, ch;
	struct timeval *tvp, waittime;
	struct itimerval itval;
	fd_set ibits;
	sigset_t sigset, osigset;

	while ((ch = getopt(argc, argv, "D012bsqtdg")) != EOF) {
		switch (ch) {
			case 'D':
				execed = 1;
				break;
			case '0': ripversion = 0; break;
			case '1': ripversion = 1; break;
			case '2': ripversion = 2; break;
			case 'b': multicast = 0; break;
			case 's': supplier = 1; break;
			case 'q': supplier = 0; break;
			case 't': 
				tflags++;
				break;
			case 'd': 
				debug++;
				setlogmask(LOG_UPTO(LOG_DEBUG));
				break;
			case 'g': gateway = 1; break;
			default:
				fprintf(stderr, "usage: routed [ -1bsqtdg ]\n");
				exit(1);
		}
	}
	
	// Modified by Mason Yu
	sleep(2);
	/*
	if(!check_pid()) {
		// Commented by Mason Yu
		//write_cfg();
		exit(1);
	}
	*/
	
	if (!execed) {
		if ((pid = vfork()) < 0) {
			fprintf(stderr, "vfork failed\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}
		
		for (_argc=0; _argc < argc; _argc++ )
			_argv[_argc] = argv[_argc];
		_argv[0] = runPath;
		_argv[argc++] = "-D";
		_argv[argc++] = NULL;
		execv(_argv[0], _argv);
		/* Not reached */
		fprintf(stderr, "Couldn't exec\n");
		_exit(1);

	} else {
		setsid();
	}

	getkversion();

	sock = getsocket();
	assert(sock>=0);

	openlog("routed", LOG_PID | LOG_ODELAY, LOG_DAEMON);

#if 0
	if (debug == 0 && tflags == 0) {
#ifndef EMBED
		switch (fork()) {
			case -1: perror("fork"); exit(1);
			case 0: break;  /* child */
			default: exit(0);   /* parent */
		}
#endif
		close(0);
		close(1);
		close(2);
		setsid();
		setlogmask(LOG_UPTO(LOG_WARNING));
	}
	else 
#endif
	{
		setlogmask(LOG_UPTO(LOG_DEBUG));
	}

	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 * 
	 * Note: because traceon() redirects stderr, anything planning to
	 * crash on startup should do so before this point.
	 */

	if (argc > 1) {
		traceon(argv[argc - 1]);
	}
	while (tflags-- > 0) {
		bumploglevel();
	}

	gettimeofday(&now, NULL);

	/*
	 * Collect an initial view of the world by
	 * checking the interface configuration and the gateway kludge
	 * file.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */
	 read_cfg();
	rtinit();
	ifinit();
	gwkludge();

	if (gateway > 0) {
		rtdefault();
	}

	if (supplier < 0) {
		supplier = 0;
	}

	signal(SIGALRM, timer);
	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
        signal(SIGTERM, terminate_routed); //RTK WiSOC modify for deleting all route entry
	signal(SIGINT, rtdeleteall);
	signal(SIGUSR1, sigtrace);
	signal(SIGUSR2, sigtrace);

	itval.it_interval.tv_sec = TIMER_RATE;
	itval.it_value.tv_sec = TIMER_RATE;
	itval.it_interval.tv_usec = 0;
	itval.it_value.tv_usec = 0;

	srandom(time(NULL) ^ getpid());

	if (setitimer(ITIMER_REAL, &itval, (struct itimerval *)NULL) < 0) {
		syslog(LOG_ERR, "setitimer: %m\n");
	}
	
	// Kaohj
//#ifdef EMBED //WiSOC we use pid file for system init
	write_pid();
//#endif
	rip_request_send();
	rip_input_init();
	DISPLAY_BANNER;
	FD_ZERO(&ibits);
	nfd = sock + 1;			/* 1 + max(fd's) */
	for (;;) {
		FD_SET(sock, &ibits);

		/*
		 * If we need a dynamic update that was held off,
		 * needupdate will be set, and nextbcast is the time
		 * by which we want select to return.  Compute time
		 * until dynamic update should be sent, and select only
		 * until then.  If we have already passed nextbcast,
		 * just poll.
		 */
		if (needupdate) {
			waittime = nextbcast;
			timevalsub(&waittime, &now);
			if (waittime.tv_sec < 0) {
				waittime.tv_sec = 0;
				waittime.tv_usec = 0;
			}
			if (traceactions)
				fprintf(ftrace,
				 "select until dynamic update %ld/%ld sec/usec\n",
				    (long)waittime.tv_sec, (long)waittime.tv_usec);
			tvp = &waittime;
		}
		else {
			tvp = (struct timeval *)NULL;
		}

		n = select(nfd, &ibits, 0, 0, tvp);
		if (n <= 0) {
			/*
			 * Need delayed dynamic update if select returned
			 * nothing and we timed out.  Otherwise, ignore
			 * errors (e.g. EINTR).
			 */
			if (n < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_ERR, "select: %m");
			}
			sigemptyset(&sigset);
			sigaddset(&sigset, SIGALRM);
			sigprocmask(SIG_BLOCK, &sigset, &osigset);
			if (n == 0 && needupdate) {
				if (traceactions)
					fprintf(ftrace,
					    "send delayed dynamic update\n");
				(void) gettimeofday(&now,
					    (struct timezone *)NULL);
				toall(supply, RTS_CHANGED,
				    (struct interface *)NULL);
				lastbcast = now;
				needupdate = 0;
				nextbcast.tv_sec = 0;
			}
			sigprocmask(SIG_SETMASK, &osigset, NULL);
			continue;
		}

		gettimeofday(&now, (struct timezone *)NULL);
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigprocmask(SIG_BLOCK, &sigset, &osigset);

		if (FD_ISSET(sock, &ibits)) {
			process(sock);
		}

		/* handle ICMP redirects */
		sigprocmask(SIG_SETMASK, &osigset, NULL);
	}
}

/*
 * Put Linux kernel version into
 * the global variable kernel_version.
 * Example: 1.2.8 = 0x010208
 */

static
void
getkversion(void)
{
	struct utsname uts;
	int maj, min, pl;

	maj = min = pl = 0;
	uname(&uts);
	sscanf(uts.release, "%d.%d.%d", &maj, &min, &pl);
	kernel_version = (maj << 16) | (min << 8) | pl;
}

void
timevaladd(struct timeval *t1, struct timeval *t2)
{

	t1->tv_sec += t2->tv_sec;
	if ((t1->tv_usec += t2->tv_usec) > 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}

void
timevalsub(struct timeval *t1, struct timeval *t2)
{

	t1->tv_sec -= t2->tv_sec;
	if ((t1->tv_usec -= t2->tv_usec) < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
}

static
void
process(int fd)
{
	struct sockaddr from;
	socklen_t fromlen;
	int cc;
	union {
		char	buf[MAXPACKETSIZE+1];
		struct	rip rip;
	} inbuf;

	for (;;) {
		fromlen = sizeof(from);
		cc = recvfrom(fd, &inbuf, sizeof(inbuf), 0, &from, &fromlen);
		if (cc <= 0) {
			if (cc < 0 && errno != EWOULDBLOCK)
				perror("recvfrom");
			break;
		}
		if (fromlen != sizeof (struct sockaddr_in)) {
			break;
		}
		rip_input(&from, &inbuf.rip, cc);
	}
}


int
setsockopt_multicast_ipv4(int sk, 
			int optname, 
			struct in_addr if_addr,
			unsigned int mcast_addr,
			unsigned int ifindex)
{

	struct ip_mreqn mreqn;
  
	switch (optname)	{
    case IP_MULTICAST_IF:
    case IP_ADD_MEMBERSHIP:
    case IP_DROP_MEMBERSHIP:
		memset (&mreqn, 0, sizeof(mreqn));

		if (mcast_addr)
			mreqn.imr_multiaddr.s_addr = mcast_addr;
      
		if (ifindex)
			mreqn.imr_ifindex = ifindex;
		else
			mreqn.imr_address = if_addr;
      
		return setsockopt(sk, IPPROTO_IP, optname, (void *)&mreqn, sizeof(mreqn));

    default:
		/* Can out and give an understandable error */
		errno = EINVAL;
		return -1;
    }
	return 0;   
}

int mcast_join(struct sockaddr *int_addr)
{
struct in_addr if_addr;
	if_addr.s_addr = satosin(*int_addr)->sin_addr.s_addr;
	return setsockopt_multicast_ipv4(sock, 
			IP_ADD_MEMBERSHIP, 
			if_addr,
			0xE0000009,
			0);
}

int mcast_leave(struct sockaddr *int_addr)
{
struct in_addr if_addr;
	if_addr.s_addr = satosin(*int_addr)->sin_addr.s_addr;
	return setsockopt_multicast_ipv4(sock, 
			IP_DROP_MEMBERSHIP, 
			if_addr,
			0xE0000009,
			0);
}

int set_mcast_if(struct sockaddr *int_addr)
{
struct in_addr if_addr;
	if_addr.s_addr = satosin(*int_addr)->sin_addr.s_addr;
	return setsockopt_multicast_ipv4(sock, 
			IP_MULTICAST_IF, 
			if_addr,
			0xE0000009,
			0);
}


/*
 * This function is called during startup, and should error to stderr,
 * not syslog, and should exit on error.
 */
static
int
getsocket(void)
{
	int s, on = 1;
	struct servent *sp;

	sp = getservbyname("router", "udp");
	if (sp == NULL) {
		fprintf(stderr, "routed: router/udp: unknown service\n");
		exit(1);
	}
	rip_port = sp->s_port;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(1);
	}

#if 1
//#ifdef SO_BROADCAST
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
		perror("setsockopt SO_BROADCAST");
		exit(1);
	}
#endif

#ifdef SO_RCVBUF
	on = BUFSPACE;
	while (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &on, sizeof(on))) {
		if (on <= 8192) {
			/* give up */
			perror("setsockopt SO_RCVBUF");
			break;
		}
		/* try 1k less */
		on -= 1024;
	}
	if (traceactions) {
		fprintf(ftrace, "recv buf %d\n", on);
	}
#endif

	addr.sin_family = AF_INET;
	addr.sin_port = rip_port;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(s);
		exit(1);
	}

	if (fcntl(s, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl O_NONBLOCK");
	}

	return (s);
}
