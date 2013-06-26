/* 
   Unix SMB/Netbios implementation.
   Version 1.5.
   Copyright (C) Andrew Tridgell 1992,1993,1994
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _SMB_H
#define _SMB_H

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 50
#endif

#ifndef MAX_OPEN_FILES
#define MAX_OPEN_FILES 50
#endif

#ifndef GUEST_ACCOUNT
#define GUEST_ACCOUNT "pcuser"
#endif

//#define BUFFER_SIZE 0xFFFF
#define BUFFER_SIZE 0x7FFF

#ifndef EXTERN
#	define EXTERN extern
#endif

#define True (0==0)
#define False (!True)

typedef unsigned char BOOL;
typedef short int16;
typedef int int32;
typedef unsigned short uint16;
typedef unsigned int uint32;

#ifndef DEF_CREATE_MASK
#define DEF_CREATE_MASK (0755)
#endif

#ifndef DEFAULT_PIPE_TIMEOUT
#define DEFAULT_PIPE_TIMEOUT 10000000 /* Ten seconds */
#endif


//#define CVAL(buf,pos) ((unsigned)(((unsigned char *)(buf))[pos]))
//#define CVAL_NC(buf,pos) (((unsigned char *)(buf))[pos]) /* Non-const version of CVAL */
//#define PVAL(buf,pos) (CVAL(buf,pos))
//#define SCVAL(buf,pos,val) (CVAL_NC(buf,pos) = (val))




/* these macros make access to the packet buffers easier. See also the
functions SSVAL() and SIVAL(). */
#define SIVAL(buf,pos,val) sival((char *)(buf),pos,val)
#define SSVAL(buf,pos,val) ssval((char *)(buf),pos,val)
#define IVAL(buf,pos) ival((char *)(buf),pos)
#define SVAL(buf,pos) sval((char *)buf,pos)
#define PVAL(buf,pos,type) (*((type *)(buf + pos)))
#define ISWP(x) (NeedSwap? uint32_byte_swap((uint32)x):(x))
#define SSWP(x) (NeedSwap? uint16_byte_swap((uint16)x):(x))
#define SCVAL(buf,pos,x) PVAL(buf,pos,unsigned char) = (x)
#define CVAL(buf,pos) PVAL(buf,pos,unsigned char)

#define BSW(x) SSWP(x)
#define BSL(x) ISWP(x)

#define DIR_STRUCT_SIZE 43

/* these define all the command types recognised by the server - there
are lots of gaps so probably there are some rare commands that are not
implemented */
#define pHELLO 114
#define pCONNECT 112
#define pFSTAT 16
#define pDIR 0x81
#define pDFREE 0x80
#define pOPEN 2
#define pCREATE 3
#define pCLOSE 4
#define pREAD 10
#define pWRITE 11
#define pMKDIR 0
#define pRMDIR 1
#define pDELETE 6
#define pGOODBYE 113
#define pACCESS 8
#define pCHMOD 9
#define pRENAME 7
#define pSETDIR '\377'

/* these define the attribute byte as seen by DOS */
#define aRONLY (1L<<0)
#define aHIDDEN (1L<<1)
#define aSYSTEM (1L<<2)
#define aVOLID (1L<<3)
#define aDIR (1L<<4)
#define aARCH (1L<<5)

/* these define some DOS error codes */
#define eINVALID_FUNCTION	1
#define eFILE_NOT_FOUND		2
#define eINVALID_PATH		3
#define eTOO_MANY_FILES_OPEN	4
#define eACCESS_DENIED		5
#define eINVALID_FILE_HANDLE	6
#define eMCB_DESTROYED		7
#define eNOT_ENOUGH_MEMORY	8
#define eMEM_ADDRESS_INVALID	9
#define eENV_STRING_INVALID	10
#define eFORMAT_INVALID		11
#define eACCESS_CODE_INVALID	12
#define eDATA_INVALID		13
#define eINVALID_DRIVE		15
#define eREMOVE_CURRENT_DIR	16
#define eNOT_SAME_DEVICE	17
#define eNO_MORE_FILES		18

#define ERRbadfile 2
#define ERRbadpath 3
#define ERRbadpw   2

typedef char pstring[MAXPATHLEN];

typedef struct
	{
	BOOL open;
	BOOL can_lock;
	int cnum;
	int fd;
	pstring name;
	} files_struct;

typedef struct
{
  BOOL open;
  int service;
  int connect_num;
  int uid;
  int gid;
  void *dirptr;
  pstring dirpath;
  pstring connectpath;
} connection_struct;


typedef struct
{
  int uid;
  char *name;
} user_struct;


/* these are useful macros for checking validity of handles */
#define VALID_FNUM(fnum)   ((fnum >= 0) && (fnum < MAX_OPEN_FILES))
#define OPEN_FNUM(fnum)    (VALID_FNUM(fnum) && Files[fnum].open)
#define VALID_CNUM(cnum)   ((cnum >= 0) && (cnum < MAX_CONNECTIONS))
#define OPEN_CNUM(cnum)    (VALID_CNUM(cnum) && Connections[cnum].open)

/* translates a connection number into a service number */
#define SNUM(cnum)         (Connections[cnum].service)

/* access various service details */
#define GUEST              (lp_guestaccount())
#define HOME(cnum)         (lp_pathname(SNUM(cnum)))
#define PATH(snum)         (lp_pathname(snum))
#define USER(snum)         (lp_username(snum))
#define SERVICE(snum)      (lp_servicename(snum))
#define PRINTCOMMAND(snum) (lp_printcommand(snum))
#define CAN_WRITE(cnum)    (OPEN_CNUM(cnum) && !lp_readonly(SNUM(cnum)))
#define VALID_SNUM(snum)   (lp_snum_ok(snum))
#define GUEST_OK(snum)     (VALID_SNUM(snum) && lp_guest_ok(snum))
#define CAN_SETDIR(snum)   (!lp_no_set_dir(snum))
#define CAN_PRINT(cnum)    (OPEN_CNUM(cnum) && lp_print_ok(SNUM(cnum)))
#define MAP_HIDDEN(cnum)   (OPEN_CNUM(cnum) && lp_map_hidden(SNUM(cnum)))
#define MAP_SYSTEM(cnum)   (OPEN_CNUM(cnum) && lp_map_system(SNUM(cnum)))
#define CREATE_MODE(cnum)  (lp_create_mode(SNUM(cnum)))

/* the basic packet size, assuming no words or bytes */
#define smb_size 39

/* this is how errors are generated */
#define ERROR(class,x) error_packet(inbuf,outbuf,class,x,__LINE__)

/* offsets into message for common items */
#define smb_com 8
#define smb_rcls 9
#define smb_reh 10
#define smb_err 11
#define smb_reb 13
#define smb_tid 28
#define smb_pid 30
#define smb_uid 32
#define smb_mid 34
#define smb_wct 36
#define smb_vwv 37
#define smb_vwv0 37
#define smb_vwv1 39
#define smb_vwv2 41
#define smb_vwv3 43
#define smb_vwv4 45
#define smb_vwv5 47
#define smb_vwv6 49
#define smb_vwv7 51
#define smb_vwv8 53
#define smb_vwv9 55
#define smb_vwv10 57
#define smb_vwv11 59


/* the complete */
#define SMBmkdir      0x00   /* create directory */
#define SMBrmdir      0x01   /* delete directory */
#define SMBopen       0x02   /* open file */
#define SMBcreate     0x03   /* create file */
#define SMBclose      0x04   /* close file */
#define SMBflush      0x05   /* flush file */
#define SMBunlink     0x06   /* delete file */
#define SMBmv         0x07   /* rename file */
#define SMBgetatr     0x08   /* get file attributes */
#define SMBsetatr     0x09   /* set file attributes */
#define SMBread       0x0A   /* read from file */
#define SMBwrite      0x0B   /* write to file */
#define SMBlock       0x0C   /* lock byte range */
#define SMBunlock     0x0D   /* unlock byte range */
#define SMBctemp      0x0E   /* create temporary file */
#define SMBmknew      0x0F   /* make new file */
#define SMBchkpth     0x10   /* check directory path */
#define SMBexit       0x11   /* process exit */
#define SMBlseek      0x12   /* seek */
#define SMBtcon       0x70   /* tree connect */
#define SMBtconX       0x75   /* tree connect and X*/
#define SMBtdis       0x71   /* tree disconnect */
#define SMBnegprot    0x72   /* negotiate protocol */
#define SMBdskattr    0x80   /* get disk attributes */
#define SMBsearch     0x81   /* search directory */
#define SMBsplopen    0xC0   /* open print spool file */
#define SMBsplwr      0xC1   /* write to print spool file */
#define SMBsplclose   0xC2   /* close print spool file */
#define SMBsplretq    0xC3   /* return print queue */
#define SMBsends      0xD0   /* send single block message */
#define SMBsendb      0xD1   /* send broadcast message */
#define SMBfwdname    0xD2   /* forward user name */
#define SMBcancelf    0xD3   /* cancel forward */
#define SMBgetmac     0xD4   /* get machine name */
#define SMBsendstrt   0xD5   /* send start of multi-block message */
#define SMBsendend    0xD6   /* send end of multi-block message */
#define SMBsendtxt    0xD7   /* send text of multi-block message */

/* Core+ protocol */
#define SMBlockread	  0x13   /* Lock a range and read */
#define SMBwriteunlock 0x14 /* Unlock a range then write */
#define SMBreadbraw   0x1a  /* read a block of data with no smb header */
#define SMBwritebraw  0x1d  /* write a block of data with no smb header */
#define SMBwritec     0x20  /* secondary write request */
#define SMBwriteclose 0x2c  /* write a file then close it */

/* dos extended protocol */
#define SMBreadBraw      0x1A   /* read block raw */
#define SMBreadBmpx      0x1B   /* read block multiplexed */
#define SMBreadBs        0x1C   /* read block (secondary response) */
#define SMBwriteBraw     0x1D   /* write block raw */
#define SMBwriteBmpx     0x1E   /* write block multiplexed */
#define SMBwriteBs       0x1F   /* write block (secondary request) */
#define SMBwriteC        0x20   /* write complete response */
#define SMBsetattrE      0x22   /* set file attributes expanded */
#define SMBgetattrE      0x23   /* get file attributes expanded */
#define SMBlockingX      0x24   /* lock/unlock byte ranges and X */
#define SMBtrans         0x25   /* transaction - name, bytes in/out */
#define SMBtranss        0x26   /* transaction (secondary request/response) */
#define SMBioctl         0x27   /* IOCTL */
#define SMBioctls        0x28   /* IOCTL  (secondary request/response) */
#define SMBcopy          0x29   /* copy */
#define SMBmove          0x2A   /* move */
#define SMBecho          0x2B   /* echo */
#define SMBopenX         0x2D   /* open and X */
#define SMBreadX         0x2E   /* read and X */
#define SMBwriteX        0x2F   /* write and X */
#define SMBsesssetup     0x73   /* Session Set Up & X (including User Logon) */
#define SMBtconX         0x75   /* tree connect and X */
#define SMBffirst        0x82   /* find first */
#define SMBfunique       0x83   /* find unique */
#define SMBfclose        0x84   /* find close */
#define SMBinvalid       0xFE   /* invalid command */


#define SUCCESS 0  /* The request was successful. */
#define ERRDOS 0x01 /*  Error is from the core DOS operating system set. */
#define ERRSRV 0x02  /* Error is generated by the server network file manager.*/
#define ERRHRD 0x03  /* Error is an hardware error. */
#define ERRCMD 0xFF  /* Command was not in the "SMB" format. */


#define ERRnosupport 0xFFFF
#define ERRinvnid 5
#define ERRaccess 4

#define ERRnoresource  89 /* No resources currently available for request. */

#ifdef USE_DIRECT
#define DIRECT direct
#else
#define DIRECT dirent
#endif


/* structure used to hold the incoming hosts info */
struct from_host {
    char   *name;			/* host name */
    char   *addr;			/* host address */
    struct sockaddr_in *sin;		/* their side of the link */
};


/* and a few prototypes */
void smb_setlen(char *buf,int len);
int set_message(char *buf,int num_words,int num_bytes);
void name_interpret(char *in,char *out);
BOOL check_access(int snum);
void exchange_uids(void );
void ssval(char *buf,int pos,uint16 val);
uint32 ival(char *buf,int pos);
BOOL string_set(char **dest,char *src);
BOOL string_init(char **dest,char *src);
void string_free(char **s);
void unix_format(char *fname);
BOOL directory_exist(char *dname);
void make_dir_struct(char *buf,char *mask,char *fname,unsigned int size,int mode,time_t date);
BOOL in_list(char *s,char *list,BOOL case_sensitive);
void strupper(char *s);
BOOL file_exist(char *fname);
int read_with_timeout(int fd,char *buf,int mincnt,int maxcnt, long time_out);
int write_socket(int fd,char *buf,int len);
void close_sockets(void );
int write_with_timeout(int fd, char *buf, int length, long time_out);
BOOL send_smb(char *buffer);
BOOL read_data(int fd,char *buffer,int N);
int smb_len(char *buf);
int receive_smb(char *buffer);
void show_msg(char *buf);
BOOL big_endian(void );
void become_daemon(void);
BOOL reduce_name(char *s,char *dir);
void strlower(char *s);
char *smb_buf();
BOOL strequal(char *,char *);
BOOL mask_match(char *,char *,BOOL );
int dos_mode(int ,char *,mode_t );
char *timestring();
uint16 sval(char *,int );
BOOL ip_equal(struct in_addr *ip1,struct in_addr *ip2);
BOOL send_packet(char *buf,int len,struct in_addr *ip,int port,int type);
char *get_home_dir(char *);
int set_filelen(int fd, long len);
uint32 make_dos_date(time_t );
int lp_keepalive(void);
int name_len(char *s);
void clean_name(char *s);
time_t make_unix_date(uint32 dos_date);
void trim_string(char *s,char *front,char *back);
int byte_checksum(unsigned char *buf,int len);
void sival(char *buf,int pos,uint32 val);
BOOL yesno(char *p);
uint32 file_size(char *file_name);
void dos_format(char *fname);
char *GetWd(char *s);
void name_mangle(unsigned char *in,unsigned char *out);
int name_len(char *s);
BOOL name_equal(char *s1,char *s2);
void show_nmb(char *inbuf);
int nmb_len(char *buf);
BOOL receive_nmb(char *buffer,int timeout);
void name_extract(char *buf,int ofs,char *name);
BOOL name_query(char *inbuf,char *outbuf,char *name,struct in_addr *ip);
BOOL get_broadcast(struct in_addr *if_ipaddr, struct in_addr *if_bcast);
BOOL allow_access(char *deny_list,char *allow_list,struct from_host *client);
void Debug();
BOOL password_ok(char *user,char *password);
int chain_reply(int type,char *inbuf,char *inbuf2,char *outbuf,char *outbuf2,int size,int bufsize);
void close_cnum(int cnum);



#ifdef SUN
struct DIRECT *readdir();
char *strcpy();
int fprintf();
#endif

char *mktemp();
int setreuid();
int setregid();
int closedir();
char *getpass();
#ifndef PWD_AUTH
#ifndef crypt
char *crypt();
#endif
#endif


#ifdef STRING_DEBUG
#define strcpy mystrcpy
#define strchr mystrchr
#define strrchr mystrrchr
#define strlen mystrlen
#define strncpy mystrncpy
#define strcat mystrcat
#define memcpy mymemcpy
#define memset mymemset
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef SIGNAL_CAST
#define SIGNAL_CAST
#endif

#ifndef SELECT_CAST
#define SELECT_CAST
#endif


/* Some POSIX definitions for those without */
 
#ifndef S_IFDIR
#define S_IFDIR         0x4000
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode)   ((mode & 0xF000) == S_IFDIR)
#endif
#ifndef S_IRWXU
#define S_IRWXU 00700           /* read, write, execute: owner */
#endif
#ifndef S_IRUSR
#define S_IRUSR 00400           /* read permission: owner */
#endif
#ifndef S_IWUSR
#define S_IWUSR 00200           /* write permission: owner */
#endif
#ifndef S_IXUSR
#define S_IXUSR 00100           /* execute permission: owner */
#endif
#ifndef S_IRWXG
#define S_IRWXG 00070           /* read, write, execute: group */
#endif
#ifndef S_IRGRP
#define S_IRGRP 00040           /* read permission: group */
#endif
#ifndef S_IWGRP
#define S_IWGRP 00020           /* write permission: group */
#endif
#ifndef S_IXGRP
#define S_IXGRP 00010           /* execute permission: group */
#endif
#ifndef S_IRWXO
#define S_IRWXO 00007           /* read, write, execute: other */
#endif
#ifndef S_IROTH
#define S_IROTH 00004           /* read permission: other */
#endif
#ifndef S_IWOTH
#define S_IWOTH 00002           /* write permission: other */
#endif
#ifndef S_IXOTH
#define S_IXOTH 00001           /* execute permission: other */
#endif


/* structures and defs for tar */
#ifndef TBLOCK
#define TBLOCK 512
#endif
#ifndef NAMSIZ
#define NAMSIZ 100
#endif
union tarhblock {
  char dummy[TBLOCK];
  struct header {
    char name[NAMSIZ];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char linkname[NAMSIZ];
  } dbuf;
};

/* protocol types. It assumes that higher protocols include lower protocols
   as subsets */
enum protocol_types {PROT_NONE,PROT_CORE,PROT_COREPLUS,PROT_LANMAN1};

#if !defined(HAVE_BOOL)
#ifdef HAVE__Bool
#define bool _Bool
#else
typedef int bool;
#endif
#endif

#define MAX_NETBIOSNAME_LEN 16
/* DOS character, NetBIOS namestring. Type used on the wire. */
typedef char nstring[MAX_NETBIOSNAME_LEN];
/* Unix character, NetBIOS namestring. Type used to manipulate name in nmbd. */
typedef char unstring[MAX_NETBIOSNAME_LEN*4];
#define MAX_DGRAM_SIZE (576) /* tcp/ip datagram limit is 576 bytes */

/* A netbios name structure. */
struct nmb_name {
	nstring      name;
	char         scope[64];
	unsigned int name_type;
};

/* A resource record. */
struct res_rec {
	struct nmb_name rr_name;
	int rr_type;
	int rr_class;
	int ttl;
	int rdlength;
	char rdata[MAX_DGRAM_SIZE];
};

/* An nmb packet. */
struct nmb_packet {
	struct {
		int name_trn_id;
		int opcode;
		bool response;
		struct {
			bool bcast;
			bool recursion_available;
			bool recursion_desired;
			bool trunc;
			bool authoritative;
		} nm_flags;
		int rcode;
		int qdcount;
		int ancount;
		int nscount;
		int arcount;
	} header;

	struct {
		struct nmb_name question_name;
		int question_type;
		int question_class;
	} question;

	struct res_rec *answers;
	struct res_rec *nsrecs;
	struct res_rec *additional;
};
#endif 


