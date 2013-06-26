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

#include "includes.h"
//pstring debugf = DEBUGFILE;
pstring debugf = "";
pstring domain="";
extern int Client;
char *InBuffer = NULL;
char *OutBuffer = NULL;

extern BOOL NeedSwap;

BOOL reply_only = False;

extern struct in_addr lastip;
extern unsigned short myPort;
extern unsigned short lastport;
extern struct in_addr myip;
extern struct in_addr bcast_ip;
pstring myname="";
pstring myhostname="";
pstring mynetifname="";
int myttl = 0x3f480;

typedef struct
{
  BOOL valid;
  pstring name;
  struct in_addr ip;
  int ttl;
  unsigned char nb_flags;
} name_struct;

int num_names=0;
name_struct *names = NULL;

void construct_reply(char *,char *);

/* are we running as a daemon ? */
BOOL is_daemon = False;


BOOL got_bcast = False;

#define SREV(x) ((((x)&0xFF)<<8) | (((x)>>8)&0xFF))	
#define RSVAL(buf,pos) SREV(SVAL(buf,pos))
#define IREV(x) ((SREV(x)<<16) | (SREV((x)>>16)))
#define RIVAL(buf,pos) IREV(IVAL(buf,pos))
#define MAX_NETBIOSNAME_LEN 16
#define SMB_MALLOC_ARRAY(type,count) (type *)malloc_array(sizeof(type),(count))
#define SAFE_FREE(x) do { if ((x) != NULL) {free(x); x=NULL;} } while(0)
#define MAX_ALLOC_SIZE (1024*1024*256)
/****************************************************************************
 Type-safe malloc.
****************************************************************************/
void *malloc_array(size_t el_size, unsigned int count)
{
	if (count >= MAX_ALLOC_SIZE/el_size) {
		return NULL;
	}

	if (el_size == 0 || count == 0) {
		return NULL;
	}
#if defined(PARANOID_MALLOC_CHECKER)
	return malloc_(el_size*count);
#else
	return malloc(el_size*count);
#endif
}
/*******************************************************************
 Allocate and parse some resource records.
******************************************************************/
static bool parse_alloc_res_rec(char *inbuf,int *offset,int length,
				struct res_rec **recs, int count)
{
	int i;

	*recs = SMB_MALLOC_ARRAY(struct res_rec, count);
	if (!*recs)
		return(False);

	memset((char *)*recs,'\0',sizeof(**recs)*count);

	for (i=0;i<count;i++) {
		int l = parse_nmb_name(inbuf,*offset,length,
				&(*recs)[i].rr_name);
		(*offset) += l;
		if (!l || (*offset)+10 > length) {
			SAFE_FREE(*recs);
			return(False);
		}
		(*recs)[i].rr_type = RSVAL(inbuf,(*offset));
		(*recs)[i].rr_class = RSVAL(inbuf,(*offset)+2);
		(*recs)[i].ttl = RIVAL(inbuf,(*offset)+4);
		(*recs)[i].rdlength = RSVAL(inbuf,(*offset)+8);
		(*offset) += 10;
		if ((*recs)[i].rdlength>sizeof((*recs)[i].rdata) ||
				(*offset)+(*recs)[i].rdlength > length) {
			SAFE_FREE(*recs);
			return(False);
		}
		memcpy((*recs)[i].rdata,inbuf+(*offset),(*recs)[i].rdlength);
		(*offset) += (*recs)[i].rdlength;
	}
	return(True);
}

/*******************************************************************
 Handle "compressed" name pointers.
******************************************************************/

static bool handle_name_ptrs(unsigned char *ubuf,int *offset,int length,
			     bool *got_pointer,int *ret)
{
	int loop_count=0;

	while ((ubuf[*offset] & 0xC0) == 0xC0) {
		if (!*got_pointer)
			(*ret) += 2;
		(*got_pointer)=True;
		(*offset) = ((ubuf[*offset] & ~0xC0)<<8) | ubuf[(*offset)+1];
		if (loop_count++ == 10 ||
				(*offset) < 0 || (*offset)>(length-2)) {
			return False;
		}
	}
	return True;
}


/*******************************************************************
 Parse a nmb name from "compressed" format to something readable
 return the space taken by the name, or 0 if the name is invalid
******************************************************************/

int parse_nmb_name(char *inbuf,int ofs,int length, struct nmb_name *name)
{
	int m,n=0;
	unsigned char *ubuf = (unsigned char *)inbuf;
	int ret = 0;
	bool got_pointer=False;
	int loop_count=0;
	int offset = ofs;

	if (length - offset < 2)
		return(0);

	/* handle initial name pointers */
	if (!handle_name_ptrs(ubuf,&offset,length,&got_pointer,&ret))
		return(0);

	m = ubuf[offset];

	if (!m)
		return(0);
	if ((m & 0xC0) || offset+m+2 > length)
		return(0);

	memset((char *)name,'\0',sizeof(*name));

	/* the "compressed" part */
	if (!got_pointer)
		ret += m + 2;
	offset++;
	while (m > 0) {
		unsigned char c1,c2;
		c1 = ubuf[offset++]-'A';
		c2 = ubuf[offset++]-'A';
		if ((c1 & 0xF0) || (c2 & 0xF0) || (n > sizeof(name->name)-1))
			return(0);
		name->name[n++] = (c1<<4) | c2;
		m -= 2;
	}
	name->name[n] = 0;

	if (n==MAX_NETBIOSNAME_LEN) {
		/* parse out the name type, its always
		 * in the 16th byte of the name */
		name->name_type = ((unsigned char)name->name[15]) & 0xff;

		/* remove trailing spaces */
		name->name[15] = 0;
		n = 14;
		while (n && name->name[n]==' ')
			name->name[n--] = 0;
	}

	/* now the domain parts (if any) */
	n = 0;
	while (ubuf[offset]) {
		/* we can have pointers within the domain part as well */
		if (!handle_name_ptrs(ubuf,&offset,length,&got_pointer,&ret))
			return(0);

		m = ubuf[offset];
		/*
		 * Don't allow null domain parts.
		 */
		if (!m)
			return(0);
		if (!got_pointer)
			ret += m+1;
		if (n)
			name->scope[n++] = '.';
		if (m+2+offset>length || n+m+1>sizeof(name->scope))
			return(0);
		offset++;
		while (m--)
			name->scope[n++] = (char)ubuf[offset++];

		/*
		 * Watch for malicious loops.
		 */
		if (loop_count++ == 10)
			return 0;
	}
	name->scope[n++] = 0;

	return(ret);
}
/*******************************************************************
 Parse a nmb packet. Return False if the packet can't be parsed
 or is invalid for some reason, True otherwise.
******************************************************************/
static bool parse_nmb(char *inbuf,int length,struct nmb_packet *nmb)
{
	int nm_flags,offset;

	memset((char *)nmb,'\0',sizeof(*nmb));

	if (length < 12)
	{
		return(False);
	}
	/* parse the header */
	nmb->header.name_trn_id = RSVAL(inbuf,0);

	//DEBUG(10,("parse_nmb: packet id = %d\n", nmb->header.name_trn_id));

	nmb->header.opcode = (CVAL(inbuf,2) >> 3) & 0xF;
	nmb->header.response = ((CVAL(inbuf,2)>>7)&1)?True:False;
	nm_flags = ((CVAL(inbuf,2) & 0x7) << 4) + (CVAL(inbuf,3)>>4);
	nmb->header.nm_flags.bcast = (nm_flags&1)?True:False;
	nmb->header.nm_flags.recursion_available = (nm_flags&8)?True:False;
	nmb->header.nm_flags.recursion_desired = (nm_flags&0x10)?True:False;
	nmb->header.nm_flags.trunc = (nm_flags&0x20)?True:False;
	nmb->header.nm_flags.authoritative = (nm_flags&0x40)?True:False;
	nmb->header.rcode = CVAL(inbuf,3) & 0xF;
	nmb->header.qdcount = RSVAL(inbuf,4);
	nmb->header.ancount = RSVAL(inbuf,6);
	nmb->header.nscount = RSVAL(inbuf,8);
	nmb->header.arcount = RSVAL(inbuf,10);

	if (nmb->header.qdcount) {
		offset = parse_nmb_name(inbuf,12,length,
				&nmb->question.question_name);
		if (!offset)
		{		
			return(False);
		}
		if (length - (12+offset) < 4)
		{		
			return(False);
		}
		nmb->question.question_type = RSVAL(inbuf,12+offset);
		nmb->question.question_class = RSVAL(inbuf,12+offset+2);

		offset += 12+4;
	} else {
		offset = 12;
	}

	/* and any resource records */
	if (nmb->header.ancount &&
			!parse_alloc_res_rec(inbuf,&offset,length,&nmb->answers,
					nmb->header.ancount))
	{	
		return(False);
	}
	if (nmb->header.nscount &&
			!parse_alloc_res_rec(inbuf,&offset,length,&nmb->nsrecs,
					nmb->header.nscount))
	{	
		return(False);
	}
	if (nmb->header.arcount &&
			!parse_alloc_res_rec(inbuf,&offset,length,
				&nmb->additional, nmb->header.arcount))
	{	
			return(False);
		}
	return(True);
}

/****************************************************************************
add a netbios name
****************************************************************************/
int add_name(void)
{
  int i;
  name_struct *n;

  for (i=0;i<num_names;i++)
    if (!names[i].valid)
      return(i);

  if (num_names == 0)    
    n = (name_struct *)malloc(sizeof(name_struct));
  else
    n = (name_struct *)realloc(names,sizeof(name_struct)*(num_names+1));
  if (!n) 
    {
      Debug(0,"Can't malloc more names space!\n");
       printf("Can't malloc more names space!\n");
      return(-1);
    }
  n[num_names].valid = False;
  names = n;
  num_names++;
  return(num_names-1);
}

/****************************************************************************
delete a netbios name
****************************************************************************/
void del_name(int i)
{
  names[i].valid = False;
}

/****************************************************************************
find a name
****************************************************************************/
int find_name(char *s)
{
  int i;
  for (i=0;i<num_names;i++)
    if (names[i].valid && name_equal(s,names[i].name))
      return(i);
  return -1;
}

/****************************************************************************
true if two IP addresses are equal
****************************************************************************/
BOOL ip_equal(struct in_addr *ip1,struct in_addr *ip2)
{
  char *p1=(char *)ip1;
  char *p2=(char *)ip2;
  int l = sizeof(*ip1);
  while (l--)
    if (*p1++ != *p2++)
      return(False);
  return(True);
}

/****************************************************************************
register a name on the net
****************************************************************************/
BOOL register_name(char *inbuf,char *outbuf,char *name,struct in_addr *ip)
{
  unsigned char nb_flags = 0;
  int count;
  static uint16 name_trn_id = 0x4242;
  char *p;
  int ret;
	
  Debug(1,"Registering name %s (%s) nb_flags=0x%x\n",
	name, inet_ntoa(*ip) ,nb_flags);

  SSVAL(outbuf,0,name_trn_id++);
  CVAL(outbuf,2) = (0x5<<3) | 0x1;
  CVAL(outbuf,3) = (1<<4) | 0x0;
  SSVAL(outbuf,4,1);
  SSVAL(outbuf,6,0);
  SSVAL(outbuf,8,0);
  SSVAL(outbuf,10,1);  
  p = outbuf+12;
  name_mangle(name,p);
  p += name_len(p);
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  p += 4;
  CVAL(p,0) = 0xC0;
  CVAL(p,1) = 12;
  p += 2;
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  SIVAL(p,4,myttl);
  SSVAL(p,8,6);
  CVAL(p,10) = nb_flags;
  CVAL(p,11) = 0;
  p += 12;
  memcpy(p,ip,4);
  p += 4;

  count = 3;
  while (count--)
    {
      Debug(2,"Sending reg request for %s at (%s)\n",name,inet_ntoa(*ip));


      show_nmb(outbuf);
      if (!send_packet(outbuf,nmb_len(outbuf),&bcast_ip,137,SOCK_DGRAM))
      {
			return False;
      }
      if (receive_nmb(inbuf,3))
	{
          int rec_name_trn_id = SVAL(inbuf,0);
	  int opcode = CVAL(inbuf,2) >> 3;
	  int nm_flags = ((CVAL(inbuf,2) & 0x7) << 4) + (CVAL(inbuf,3)>>4);
	  int rcode = CVAL(inbuf,3) & 0xF;

	  /* is it a negative response to our request? */
	  if ((rec_name_trn_id = name_trn_id) && 
              (opcode == 5 && nm_flags == 0x58 && rcode != 0 && rcode != 7))
	    {
	      char qname[100];
	      name_extract(inbuf,12,qname);
	      if (name_equal(qname,name))
		{
		  Debug(0,"Someone (%s) gave us a negative name regregistration response!\n",
			inet_ntoa(lastip));
		  return False;
		}
	    }	  
	  
	  /* it's something else - process it anyway, unless we are running
	   as a daemon. This is necessary as we may have been started by a 
	   name query of our name arriving on port 137 (often happens) */
	  if (!is_daemon)
	    {
	      show_nmb(inbuf);
	      construct_reply(inbuf,outbuf + nmb_len(outbuf));
	    }
	}
    }

  /* no negative replies, send a demand */
  p = outbuf;
  SSVAL(outbuf,0,name_trn_id++);
  CVAL(outbuf,2) = (0x5<<3);
  CVAL(outbuf,3) = (1<<4) | 0x0;
  SSVAL(outbuf,4,1);
  SSVAL(outbuf,6,0);
  SSVAL(outbuf,8,0);
  SSVAL(outbuf,10,1);  
  p = outbuf+12;
  name_mangle(name,p);
  p += name_len(p);
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  p += 4;
  CVAL(p,0) = 0xC0;
  CVAL(p,1) = 12;
  p += 2;
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  SIVAL(p,4,myttl);
  SSVAL(p,8,6);
  CVAL(p,10) = nb_flags;
  CVAL(p,11) = 0;
  p += 12;
  memcpy(p,ip,4);
  p += 4;
  
  Debug(2,"Sending reg demand for %s at (%s)\n",name,inet_ntoa(*ip));

  show_nmb(outbuf);
  ret = (send_packet(outbuf,nmb_len(outbuf),&bcast_ip,137,SOCK_DGRAM));
  if(!ret)
  {
	  //printf("%s:111 send_packet error!!\n", __FUNCTION__);
  }
  return ret;
}


/****************************************************************************
reply to a reg request
****************************************************************************/
void reply_reg_request(char *inbuf,char *outbuf)
{
  int name_trn_id = SVAL(inbuf,0);
  char qname[100]="";
  int ttl;
  char *p = inbuf;
  struct in_addr ip;
  int n=0;
  unsigned char nb_flags;
  int ret;
  
  name_extract(inbuf,12,qname);

  p += 12;
  p += name_len(p);
  p += 4;
  p += name_len(p);
  p += 4;
  ttl = IVAL(p,0);
  nb_flags = CVAL(p,6);
  p += 8;
  memcpy(&ip,p,4);

  Debug(2,"Name registration request for %s (%s) nb_flags=0x%x\n",
	qname,inet_ntoa(ip),nb_flags);

  /* if the name doesn't exist yet then don't respond */
  if ((n = find_name(qname)) < 0)
    return;

  /* if it's a group name and being registered as a group then it's OK */
  if ((names[n].nb_flags & 0x80) && (nb_flags & 0x80))
    return;

  /* if it's not my name then don't worry about it */
  if (!name_equal(myname,qname))
    return;

  Debug(0,"Someones using my name (%s), sending negative reply\n",qname);

  /* Send a NEGATIVE REGISTRATION RESPONSE to protect our name */
  SSVAL(outbuf,0,name_trn_id);
  CVAL(outbuf,2) = (1<<7) | (0x5<<3) | 0x5;
  CVAL(outbuf,3) = (1<<7) | 0x6;
  SSVAL(outbuf,4,0);
  SSVAL(outbuf,6,1);
  SSVAL(outbuf,8,0);
  SSVAL(outbuf,10,0);  
  p = outbuf+12;
  strcpy(p,inbuf+12);
  p += name_len(p);
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  SIVAL(p,4,names[n].ttl);
  SSVAL(p,8,6);
  CVAL(p,10) = nb_flags;
  CVAL(p,11) = 0;
  p += 12;

  memcpy(p,&ip,4);      /* IP address of the name's owner (that's us) */
  p += 4;

  if (ip_equal(&ip,&bcast_ip))
    {
      Debug(0,"Not replying to broadcast address\n");
      return;
    }

  show_nmb(outbuf);
  ret = send_packet(outbuf,nmb_len(outbuf),&ip,137,SOCK_DGRAM);
  if(!ret)
  {
	  //printf("%s:send_packet error!!\n", __FUNCTION__);
  }
  return;
}

/****************************************************************************
reply to a name query
****************************************************************************/
void reply_name_query(char *inbuf,char *outbuf)
{
  int name_trn_id = SVAL(inbuf,0);
  char qname[100]="";
  char *p = inbuf;
  unsigned char nb_flags = 0;
  struct in_addr tmpip;
  int tmpport;
  int ret;
  
  name_extract(inbuf,12,qname);

  /* if it's not my name then return */
  if (!name_equal(qname,myname)){
    return;
}
  Debug(2,"(%s) querying my name (%s), sending positive reply\n",
	inet_ntoa(lastip), qname);
// printf("%s:(%s) querying my name (%s), sending positive reply\n",	__FUNCTION__,inet_ntoa(lastip), qname);
  /* Send a POSITIVE NAME QUERY RESPONSE to ack our presence */
  SSVAL(outbuf,0,name_trn_id);
  CVAL(outbuf,2) = (1<<7) | 0x4;

  CVAL(outbuf,3) = 0;
  SSVAL(outbuf,4,0);
  SSVAL(outbuf,6,1);
  SSVAL(outbuf,8,0);
  SSVAL(outbuf,10,0);  
  p = outbuf+12;
  strcpy(p,inbuf+12);
  p += name_len(p);
  SSVAL(p,0,0x20);
  SSVAL(p,2,0x1);
  SIVAL(p,4,myttl);
  SSVAL(p,8,6);
  CVAL(p,10) = nb_flags;
  CVAL(p,11) = 0;
  p += 12;
  memcpy(p,&myip,4);
  p += 4;

  show_nmb(outbuf);

  tmpip = lastip;
  tmpport = lastport;
  ret = send_packet(outbuf,nmb_len(outbuf),&tmpip,tmpport,SOCK_DGRAM);
  if(!ret)
  {
	  //printf("%s:send_packet error!!\n", __FUNCTION__);
  }
  return;
}

/****************************************************************************
reply to a name query
****************************************************************************/
void reply_name_query_status(char *inbuf,char *outbuf)
{
	struct in_addr tmpip;
	char tmpBuf[24];
	unsigned char mac[6];
	size_t len;
	int i;
	int ret;
	int tmpport;	
	char *p;
	char *pTemp;
	int name_trn_id = SVAL(inbuf,0);
	struct nmb_packet orig_nmbBuf;
	parse_nmb(inbuf,nmb_len(inbuf),&orig_nmbBuf);

	struct ifreq ifr;
	bzero(mac, sizeof(mac));
	strncpy(ifr.ifr_name, "br0", IFNAMSIZ - 1);
	if (ioctl(Client, SIOCGIFHWADDR, &ifr) == -1)
	{
		printf("ioctl error");
	}
	else
	{
		memcpy(mac,ifr.ifr_hwaddr.sa_data,6);
	}
	SSVAL(outbuf,0,name_trn_id);
	CVAL(outbuf,2) = (1<<7) | 0x4;
	CVAL(outbuf,3) = 0;
	SSVAL(outbuf,4,0);
	SSVAL(outbuf,6,1);
	SSVAL(outbuf,8,0);
	SSVAL(outbuf,10,0);  
	p = outbuf+12;
	strcpy(p,inbuf+12);
	p += name_len(p);
	SSVAL(p,0,0x21);
	SSVAL(p,2,0x1);
	SIVAL(p,4,0);
	SSVAL(p,8,65);//namenum*18+1+46 //namenum=1
	CVAL(p,10) = 1; //namenum	
	p += 11;
	len = strlen(myname);
	pTemp = myname;
	for(i=0;i<len;i++)
	{
		tmpBuf[i] = toupper(*pTemp);
		pTemp++;
	}	
	tmpBuf[len]='\0';
	len = strlen(tmpBuf);
	memset(tmpBuf + len, ' ', MAX_NETBIOSNAME_LEN - len - 1);
	tmpBuf[MAX_NETBIOSNAME_LEN - 1] = '\0';
	tmpBuf[15] = ' ';//name_type;
	tmpBuf[16] = 0x4; //NB_ACTIVE
	tmpBuf[17] = '\0';
	memcpy(p, tmpBuf, 18);
	p += 18;
	memcpy(p,mac,6);//mac addr
	p += 6;	
	CVAL(p,0) = 0; 		//Jumpers	
	CVAL(p,1) = 0; 		//test result
	SSVAL(p,2,0x00);	//version number
	SSVAL(p,4,0x00);	//period of statistics
	SSVAL(p,6,0x00);	//CRCS
	SSVAL(p,8,0x00);	//ERRORS
	SSVAL(p,10,0x00);	//COLLISIONS
	SSVAL(p,12,0x00);	//SEND ABORT
	SIVAL(p,16,0x00);	//GOOD SENDS
	SIVAL(p,20,0x00);	//GOOD RECEIVES
	SSVAL(p,22,0x00);	// retransmits
	SSVAL(p,24,0x00);	//no resource conditions
	SSVAL(p,26,0x00);	//command blocks
	SSVAL(p,28,0x00);	//pending sessions
	SSVAL(p,30,0x00);	//max number of pending sessions
	SSVAL(p,32,0x00);	//max total sessions possible
	SSVAL(p,34,0x00);	//session data packet size
	p += 36;
	len = p-outbuf;
	tmpip = lastip;
	tmpport = lastport;
	len = 247;
	ret = send_packet(outbuf,len,&tmpip,tmpport,SOCK_DGRAM);
	if(!ret)
	{
		//printf("%s:send_packet error!!\n", __FUNCTION__);
	}
	return;
}
/****************************************************************************
  construct a reply to the incoming packet
****************************************************************************/
void construct_reply(char *inbuf,char *outbuf)
{

  int opcode = (CVAL(inbuf,2) >> 3) & 0xF;
  int nm_flags = ((CVAL(inbuf,2) & 0x7) << 4) + (CVAL(inbuf,3)>>4);
  int rcode = CVAL(inbuf,3) & 0xF;
  
  int length;
  struct nmb_packet nmb;
  bzero((char*)&nmb, sizeof(nmb));
  length = nmb_len(inbuf);
  parse_nmb(inbuf,length,&nmb);
  
   if (opcode == 0 && (nm_flags&~1) == 0x10 && rcode == 0 && nmb.question.question_type == 0x2000){
    		reply_name_query(inbuf,outbuf);
	}
	if(opcode == 0 && nmb.question.question_type == 0x2100)
	{
		reply_name_query_status(inbuf,outbuf);
	}
  /*
  if (opcode == 0x5 && (nm_flags & ~1) == 0x10 && rcode == 0){
  	printf("will call reply_reg_request\n");
    reply_reg_request(inbuf,outbuf);
}
  if (opcode == 0 && (nm_flags&~1) == 0x10 && rcode == 0){
  	printf("will call reply_name_query\n");
    reply_name_query(inbuf,outbuf);
}*/
}


/****************************************************************************
  process commands from the client
****************************************************************************/
void process(char *lookup)
{
  static int trans_num = 0;
  InBuffer = (char *)malloc(BUFFER_SIZE);
  OutBuffer = (char *)malloc(BUFFER_SIZE);
  	if ((InBuffer == NULL) || (OutBuffer == NULL)) {
   	 return;
	}
  if (!reply_only && !register_name(InBuffer,OutBuffer,myname,&myip))
    {
      Debug(0,"Failed to register my own name\n");
      return;
    }
  if (*lookup)
    {
      struct in_addr ip;
      if (name_query(InBuffer,OutBuffer,lookup,&ip))
		printf("%s %s\n",inet_ntoa(ip),lookup);
      else
		printf("couldn't find name %s\n",lookup);
      return;
    }
  while (True)
    {
      if (!receive_nmb(InBuffer,0)){	
		return;
	}
      show_nmb(InBuffer);

      Debug(2,"%s Transaction %d\n",timestring(),trans_num);
      construct_reply(InBuffer,OutBuffer);

      trans_num++;
    }
}


/****************************************************************************
  open the socket communication
****************************************************************************/
BOOL open_sockets(BOOL daemon,int port)
{
  
  int val = 1;
  if (daemon)
    {
      struct hostent *hp;
      struct sockaddr_in sock;
      /* get host info */
      //
      //if ((hp = gethostbyname(myhostname)) == 0) 
//	{
//	  Debug(0, "Gethostbyname: Unknown host. %s\n",myhostname);
//	  return False;
//	}
  
      memset(&sock, 0, sizeof(sock));
      //memcpy(&sock.sin_addr, hp->h_addr, hp->h_length);
      memcpy(&sock.sin_addr,  &myip, 4);
     
      sock.sin_port = htons( port );
      sock.sin_family = AF_INET;//hp->h_addrtype;
      sock.sin_addr.s_addr = htonl(INADDR_ANY);
      Client = socket(PF_INET, SOCK_DGRAM, 0);
      if (Client == -1) 
	{
	  perror("socket");
	  return False;
	}

	if( setsockopt(Client,SOL_SOCKET,SO_REUSEADDR,(char *)&val,sizeof(val)) == -1 ) {
		close(Client);
	  	return False;
	}
  
  	memset(&sock, 0, sizeof(struct sockaddr_in));
    	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);//PORT=>ssdpPort
	myPort = port;
	/* NOTE : it seems it doesnt work when binding on the specific address */
    	/*sockname.sin_addr.s_addr = inet_addr(UPNP_MCAST_ADDR);*/
    	sock.sin_addr.s_addr = htonl(INADDR_ANY);
    	/*sockname.sin_addr.s_addr = inet_addr(ifaddr);*/
      /* now we've got a socket - we need to bind it */
      while (bind(Client, (struct sockaddr * ) &sock,sizeof(sock)) < 0) 
	{
	  perror("bind");
	  close(Client);
	  return False;
	}
    }
  else
    {
      Client = 0;
    }
  /* We will abort gracefully when the client or remote system 
     goes away */
  signal(SIGPIPE, SIGNAL_CAST abort);
  return True;
}


/****************************************************************************
  initialise connect, service and file structs
****************************************************************************/
BOOL init_structs(void )
{
	struct ifreq ifr; 
	struct ifreq ifr1; 
	int   fd=0;  
	struct sockaddr_in *addr; 
	struct sockaddr_in *sin;
  	int i = add_name();  
  	struct hostent *hp;

	if (i < 0){
	 return(False);
	}
	strcpy(names[i].name,domain);
	memset(&names[i].ip,0,4);
	names[i].ttl = 0;
	names[i].nb_flags = 0x80;
	names[i].valid = True;
  
  
	fd = socket(AF_INET, SOCK_DGRAM, 0); 
	if (fd == -1) {
	return 0; 
	}
	memset(&ifr, 0x00, sizeof(ifr));  
	if(*mynetifname==0)
		strcpy(mynetifname,"br0");
	strcpy(ifr.ifr_name, mynetifname); 
	if(ioctl(fd, SIOCGIFFLAGS, (char *)&ifr)<0){
		close( fd );
		return (0);
	}

     if (ifr.ifr_ifru.ifru_flags & (IFF_RUNNING | IFF_UP)){
           if(ioctl(fd, SIOCGIFADDR, (char *)&ifr)<0){
    		close( fd );
		return (0);
		}
            sin = (struct sockaddr_in *)&ifr.ifr_addr;  
            if (sin->sin_addr.s_addr){
            		memcpy(&myip.s_addr,&sin->sin_addr.s_addr,4);
       	 }
      } 
 	close(fd);    
 	if(*myname == 0)
 	{
 		if (gethostname(myname, sizeof(myname)) == -1) 
	    {
	      fprintf(stderr,"gethostname failed\n");
	      Debug(0,"gethostname failed\n");
	      return False;
	    } 
 	}
 	
 	sprintf(myhostname, "%s", myname);
 	//printf("myname=%s, my ip=%08X\n",myname, sin->sin_addr.s_addr);
  /* get my host name */
/*  
  if (gethostname(myhostname, sizeof(myhostname)) == -1) 
    {
      Debug(0,"gethostname failed\n");
      return False;
    } 
*/
  /* get host info */
/*  
  if ((hp = gethostbyname(myhostname)) == 0) 
    {
      Debug(0, "Gethostbyname: Unknown host %s.\n",myhostname);
      return False;
    }
  memcpy(&myip,hp->h_addr,4);

*/


  /* Read the broadcast address from the interface */
  if (!got_bcast)
    get_broadcast(&myip,&bcast_ip);

  //if (*myname == 0)
  //  strcpy(myname,myhostname);
  return True;
}
static void signal_handler(int sig_no)
{
	int i;
	name_struct *n;
	//printf("signal catched, code %d\n", sig_no);
	if(sig_no==SIGTERM){
		if(InBuffer!=NULL){
			free(InBuffer);
		}
		if(OutBuffer!=NULL){
			free(OutBuffer);
		}
		if(Client != -1 && Client != 0)
			close(Client);
			n=names;
		 for (i=0;i<num_names;i++){
		 	if(n != NULL){
		 		free(n);
		 	}
		 	n = n+i;
		}
	}
}

static void set_signal(void)
{
	struct sigaction act;

	bzero(&act, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	if(sigaction(SIGTERM, &act, NULL)){
		printf("Failed to set SIGTERM handler\n");
	}
	if(sigaction(SIGINT, &act, NULL)){
		printf("Failed to set SIGINT handler\n");
	}
	
}



/****************************************************************************
usage on the program
****************************************************************************/
void usage(char *pname)
{
  printf("Usage: %s [-n name] [-B bcast address] [-D] [-p port] [-d debuglevel] [-l log basename]\n",pname);
  printf("\t-D                    become a daemon\n");
  printf("\t-P                    passive only. don't respond\n");
  printf("\t-R                    only reply to queries, don't actively send claims\n");
  printf("\t-p port               listen on the specified port\n");
  printf("\t-d debuglevel         set the debuglevel\n");
  printf("\t-l log basename.      Basename for log/debug files\n");
  printf("\t-n netbiosname.       the netbios name to advertise for this host,default use linux hostname\n");
  printf("\t-i interface.         the net interface to run nmbserver on,default br0\n");
  printf("\t-B broadcast address  the address to use for broadcasts\n");
  printf("\t-L name              lookup this netbios name then exit\n");
  printf("\n");
}


/****************************************************************************
  main program
****************************************************************************/
int main(int argc,char *argv[])
{
  int port = 137;
  int opt;
  extern FILE *dbf;
  extern int DEBUGLEVEL;
  extern char *optarg;
  pstring lookup = "";
  while ((opt = getopt (argc, argv, "L:B:Rn:i:l:d:Dp:hP")) != EOF)
    switch (opt)
      {
      case 'B':
	{
	  unsigned long a = inet_addr(optarg);
	  memcpy(&bcast_ip,&a,sizeof(a));
	  got_bcast = True;
	}
	break;
      case 'n':
	strcpy(myname,optarg);
	break;
	  case 'i':
	  	strcpy(mynetifname,optarg);
	  	break;
      case 'P':
	{
	  extern BOOL passive;
	  passive = True;
	}
	break;
      case 'R':
	reply_only = True;
	break;
      case 'l':
	strcpy(debugf,optarg);
	break;
      case 'L':
	strcpy(lookup,optarg);
	break;
      case 'D':
	is_daemon = True;
	break;
      case 'd':
	DEBUGLEVEL = atoi(optarg);
	break;
      case 'p':
	port = atoi(optarg);
	myPort = port;
	break;
      case 'h':
	usage(argv[0]);
	exit(0);
	break;
      default:
	usage(argv[0]);
	exit(1);
      }

  /* NOTE: This is the opposite of the smbserver as name packets
     seem to use the opposite byte order to smb packets */
  NeedSwap = !big_endian();
  if (DEBUGLEVEL > 2)
    {
      extern FILE *login,*logout;
      pstring fname="";
      sprintf(fname,"%s.nmb.in",debugf);
      login = fopen(fname,"w"); 
      sprintf(fname,"%s.nmb.out",debugf);
      logout = fopen(fname,"w");
    }
  
  if (DEBUGLEVEL > 0)
    {
      pstring fname="";
      sprintf(fname,"%s.nmb.debug",debugf);      
      dbf = fopen(fname,"w");
      setbuf(dbf,NULL);
      Debug(1,"%s netbios nameserver version %s started\n",timestring(),VERSION);
      Debug(1,"Copyright Andrew Tridgell 1992,1993,1994\n");
    }
  
 
  
  if (is_daemon)
    {
    
      Debug(2,"%s becoming a daemon\n",timestring());
      //become_daemon();
	      if (daemon(0,1) < 0){
			printf("daemonizing failed\n");
			exit(1);
	       }
	
    }
    
 (void)init_structs();
 
 set_signal();
  if (open_sockets(is_daemon,port))
    {
      process(lookup);
      close_sockets();
    }
  fclose(dbf);
  
  return(0);
}

#ifndef _LOADPARM_H
/* This is a dummy lp_keepalive() for the nameserver only */
int lp_keepalive()
{
return(0);
}
#endif
