#include "local.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <pwd.h>
#include <unistd.h>


#ifdef LINUX
#include <net/if.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <sys/vfs.h>
#define SIGNAL_CAST (__sighandler_t)
#endif

#ifdef SUN
#include <net/if.h>
#include <sys/dirent.h>
#include <sys/acct.h>
#include <sys/vfs.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#ifdef sun386
/* Things we need to change for sun386i */
#ifdef PWDAUTH
#undef PWDAUTH
#endif
#define strerror strerror
struct utimbuf {
  time_t actime;
  time_t modtime;
};
typedef unsigned short mode_t;
void abort();
#endif
#ifndef strerror
extern char *sys_errlist[];
#define strerror(i) sys_errlist[i]
#endif
#endif

#ifdef SOLARIS
#include <net/if.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <sys/acct.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <sys/filio.h>
#include <string.h>
#define SYSV
#endif

#ifdef SVR4
#include <net/if.h>
#include <string.h>
#include <sys/dir.h>
#include <dirent.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>
#include <sys/filio.h>
#include <fcntl.h>
#include <sys/sockio.h>
#include <termios.h>
#define SYSV
#endif

#ifdef ULTRIX
#include <strings.h>
#include <nfs/nfs_clnt.h>
#include <nfs/vfs.h>
#include <net/if.h>
char *getwd(char *);
#endif

#ifdef OSF1
#include <strings.h>
#include <dirent.h>
#include <net/if.h>
char *getwd(char *);
#endif

#ifdef BSDI

#endif

#ifdef NETBSD
#include <net/if.h>
#include <strings.h>
#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif 

struct spwd { /* fake shadow password structure */
       char *sp_pwdp;
};
struct spwd *getspnam(char *username); /* fake shadow password routine */
#endif 



#ifdef AIX
#include <net/if.h>
#include <strings.h>
#include <sys/dir.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#endif

#ifdef HPUX
#include <net/if.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <sys/types.h>
#define SIGNAL_CAST (void (*)(__harg))
#define SELECT_CAST (int *)
#endif

#ifdef SEQUENT
char *strchr();
char *strrchr();
typedef int mode_t;
#define SEEK_SET 0
#endif

#ifdef USE_DIRECT
#include <sys/dir.h>
#endif
#define Undefined (-1)
//#define False (0)
//#define True (1)
#define Auto (2)
#define Required (3)

//#ifndef _BOOL
//typedef int BOOL;
//#define _BOOL       /* So we don't typedef BOOL again in vfs.h */
//#endif
#include "version.h"
#include "smb.h"

