
/*

    File: dns.c
    
    Copyright (C) 1999 by Wolfgang Zekoll <wzk@quietsche-entchen.de>

    This source is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This source is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns.h"
#include "lib.h"
#include "common.h"

/*
static int get_objectname(unsigned char *msg, unsigned const char *limit, 
			  unsigned char **here, char *string, int strlen,
			  int k);
*/

static int get_objname(unsigned char buf[], const int bufsize, int *here,
		      char name[], const int namelen) {
  int count=1000;
  if (*here > bufsize) return 0;
  
}

int free_packet(dnsheader_t *x)
{
    free(x->packet);
    free(x);
    return (0);
}

static dnsheader_t *alloc_packet(void *packet, int len)
{
    dnsheader_t *x;

    x = malloc(sizeof(dnsheader_t));
    memset(x, 0, sizeof(dnsheader_t));

    x->packet = malloc(len + 2);
    x->len    = len;
    memcpy(x->packet, packet, len);

    return (x);
}

static dnsheader_t *decode_header(void *packet, int len)
{
    unsigned short int *p;
    dnsheader_t *x;

    x = alloc_packet(packet, len);
    p = (unsigned short int *) x->packet;

    x->id      = ntohs(p[0]);
    x->u       = ntohs(p[1]);
    x->qdcount = ntohs(p[2]);
    x->ancount = ntohs(p[3]);
    x->nscount = ntohs(p[4]);
    x->arcount = ntohs(p[5]);

    x->here    = (char *) &x->packet[12];
    return (x);
}

static int raw_dump(dnsheader_t *x)
{
    unsigned int c;
    int	start, i, j;

    start = x->here - x->packet;
    for (i = 0; i < x->len; i += 16) {
	fprintf(stderr, "%03X -", i);

	for (j = i; j < x->len  &&  j < i+16; j++) {
	    fprintf(stderr, " %02X", ((unsigned int) x->packet[j]) & 0XFF);
	}
	for (; j < i+16; j++) {
	    fprintf(stderr, "   ");
	}

	fprintf(stderr, "  ");
	for (j = i; j < x->len  &&  j < i+16; j++) {
	    c = ((unsigned int) x->packet[j]) & 0XFF;
	    fprintf(stderr, "%c", (c >= 32  &&  c < 127) ? c : '.');
	}
	
	fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");
    return (0);
}

/*
static int get_objname(unsigned char buf[], const int bufsize, int *here,
		      char name[], const int namelen) {
  int i,p=*here, count=1000;
  unsigned int len, offs;
  if (p > bufsize) return 0;
  while (len = buf[p]) {
    
    while (len & 0x0c) {
      if (++p > bufsize) return 0;
      offs = lenbuf[p] 

}
*/


static int get_objectname(unsigned char *msg, unsigned const char *limit, 
			  unsigned char **here, char *string, int strlen,
			  int k)
{
    unsigned int len;
    int	i;

    if ((*here>=limit) || (k>=strlen)) return(-1);
    while ((len = **here) != 0) {

	*here += 1;
	if ( *here >= limit ) return(-1);
	/* If the name is compressed (see 4.1.4 in rfc1035)  */
	if (len & 0xC0) {
	    unsigned offset;
	    unsigned char *p;

	    offset = ((len & ~0xc0) << 8) + **here;
	    if ((p = &msg[offset]) >= limit) return(-1);
	    if (p == *here-1) {
	      log_debug("looping ptr");
	      return(-2);
	    }

	    if ((k = get_objectname(msg, limit, &p, string, RR_NAMESIZE, k))<0)
	      return(-1); /* if we cross the limit, bail out */
	    break;
	}
	else if (len < 64) {
	  /* check so we dont pass the limits */
	  if (((*here + len) > limit) || (len+k+2 > strlen)) return(-1);

	  for (i=0; i < len; i++) {
	    string[k++] = **here;
	    *here += 1;
	  }

	  string[k++] = '.';
	}
    }

    *here += 1;
    string[k] = 0;
    
    return (k);
}


static unsigned char *read_record(dnsheader_t *x, rr_t *y,
				  unsigned char *here, int question,
				  unsigned const char *limit)
{
    int	k, len;
    unsigned short int conv;
	unsigned short check_len=0;
    /*
     * Read the name of the resource ...
     */

    k = get_objectname(x->packet, limit, &here, y->name, sizeof(y->name), 0);
    if (k < 0) return(NULL);
    y->name[k] = 0;

    /* safe to read TYPE and CLASS? */
    if ((here+4) > limit) return (NULL);

    /*
     * ... the type of data ...
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->type = ntohs(conv);
    here += 2;

    /*
     * ... and the class type.
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->class = ntohs(conv);
    here += 2;

    /*
     * Question blocks stop here ...
     */

    if (question != 0) return (here);


    /*
     * ... while answer blocks carry a TTL and the actual data.
     */

    /* safe to read TTL and RDLENGTH? */
    if ((here+6) > limit) return (NULL);
    memcpy(&y->ttl, here, sizeof(unsigned long int));
    y->ttl = ntohl(y->ttl);
    here += 4;

    /*
     * Fetch the resource data.
     */

    //memcpy(&y->len, here, sizeof(unsigned short int));
    memcpy(&check_len, here, sizeof(unsigned short));
    printf("the check len=%X\n", check_len);
    printf("A the len =%X\n", y->len);
    len = y->len = ntohs(y->len);
    here += 2;

    /* safe to read RDATA? */
    if ((here + y->len) > limit) return (NULL);
    
    if (y->len > sizeof(y->data) - 4) {
	y->len = sizeof(y->data) - 4;
    }

    memcpy(y->data, here, y->len);
    here += len;
    y->data[y->len] = 0;

    return (here);
}


int dump_dnspacket(char *type, unsigned char *packet, int len)
{
  int	i;
  rr_t	y;
  dnsheader_t *x;
  unsigned char *limit;
  
  if (opt_debug < 2) return 0;

  if ((x = decode_header(packet, len)) == NULL ) {
    return (-1);
  }
  limit = x->packet + len;

  if (x->u & MASK_Z) log_debug("Z is set");

  fprintf(stderr, "\n");
  fprintf(stderr, "- -- %s\n", type);
  raw_dump(x);
  
  fprintf(stderr, "\n");
  fprintf(stderr, "id= %u, q= %d, opc= %d, aa= %d, wr/ra= %d/%d, "
	  "trunc= %d, rcode= %d [%04X]\n",
	  x->id, GET_QR(x->u), GET_OPCODE(x->u), GET_AA(x->u),
	  GET_RD(x->u), GET_RA(x->u), GET_TC(x->u), GET_RCODE(x->u), x->u);

  fprintf(stderr, "qd= %u\n", x->qdcount);
  
  if ((x->here = read_record(x, &y, x->here, 1, limit)) == NULL) {
    free_packet(x);
    return(-1);
  }

  fprintf(stderr, "  name= %s, type= %d, class= %d\n",
	  y.name, y.type, y.class);
  
  fprintf(stderr, "ans= %u\n", x->ancount);
  for (i = 0; i < x->ancount; i++) {
    if ((x->here = read_record(x, &y, x->here, 0, limit)) == NULL) {
      free_packet(x);
      return(-1);
    }
    fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
	    y.name, y.type, y.class, y.ttl);
  }
	    
  fprintf(stderr, "ns= %u\n", x->nscount);
  for (i = 0; i < x->nscount; i++) {
    if ((x->here = read_record(x, &y, x->here, 0, limit))==NULL) {
      free_packet(x);
      return(-1);
    }
    fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
	    y.name, y.type, y.class, y.ttl);
  }
    
  fprintf(stderr, "ar= %u\n", x->arcount);
  for (i = 0; i < x->arcount; i++) {
    if ((x->here = read_record(x, &y, x->here, 0, limit))==NULL) {
      free_packet(x);
      return(-1);
    }
    fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
	    y.name, y.type, y.class, y.ttl);
  }
	    
  fprintf(stderr, "\n");
  free_packet(x);

  	return 0;
}



dnsheader_t *parse_packet(unsigned char *packet, int len)
{
    dnsheader_t *x;

    x = decode_header(packet, len);
    return (x);
}

/*
int get_dnsquery(dnsheader_t *x, rr_t *query)
{
    char	*here;

    if (x->qdcount == 0) {
	return (1);
    }

    here = &x->packet[12];
    read_record(x, query, here, 1);

    return (0);
}
*/

/*
 * parse_query()
 *
 * The function get_dnsquery() returns us the query part of an
 * DNS packet.  For this we must already have a dnsheader_t
 * packet which is additional work.  To speed things a little
 * bit up (we use it often) parse_query() gets the query direct.
 */
unsigned char *parse_query(rr_t *y, unsigned char *msg, int len)
{
    int	k;
    unsigned char *here;
    unsigned short int conv;
    unsigned const char *limit = msg+len; 
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_ULINKER)
    extern      char HOSTNAME_CK[];
#endif
    /* If QDCOUNT, the number of entries in the question section,
     * is zero, we just give up */

    if (ntohs(((dnsheader_t *)msg)->qdcount) == 0 ) {
      log_debug("QDCOUNT was zero");
      return(0);
    }

    /*
    if (ntohs(((dnsheader_t *)msg)->u) & MASK_QR ) {
      log_debug("QR bit set. This is a reponse?");
      return(0);
    }
    */

    y->flags = ntohs(((short int *) msg)[1]);

    here = &msg[PACKET_DATABEGIN];
    if ((k=get_objectname(msg, limit, &here, y->name, sizeof(y->name),0))< 0) 
      return(0);
    y->name[k] = 0;

    /* check that there is type and class */
    if (here + 4 > limit) return(0);
    memcpy(&conv, here, sizeof(unsigned short int));
    y->type = ntohs(conv);
    here += 2;

    memcpy(&conv, here, sizeof(unsigned short int));
    y->class = ntohs(conv);
    here += 2;

    /* those strings should have been checked in get_objectname */
    k = strlen(y->name);
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_ULINKER)
//Brad save the original name before convert to lower case
	if(k > 50){
		snprintf(HOSTNAME_CK, 50, "%s", y->name);	
	}else{
		sprintf(HOSTNAME_CK, "%s",y->name); 
	}
#endif     
    if (k > 0  &&  y->name[k-1] == '.') {
	y->name[k-1] = '\0';
    }

    /* should we really convert the name to lowercase?  
     * rfc1035 2.3.3
     */

    strlwr(y->name);
	    
    return (here);
}

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_ULINKER)
static unsigned char *read_record1(dnsheader_t *x, rr_t *y,
				  unsigned char *here, int question,
				  unsigned const char *limit)
{
    int	k, len;
    unsigned short int conv;
	unsigned short check_len=0;
    /*
     * Read the name of the resource ...
     */

    k = get_objectname(x->packet, limit, &here, y->name, sizeof(y->name), 0);
    if (k < 0) {
    	return(NULL);
	}
    y->name[k] = 0;

    /* safe to read TYPE and CLASS? */
    if ((here+4) > limit){
     	return (NULL);
	}
    /*
     * ... the type of data ...
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->type = ntohs(conv);
    here += 2;

    /*
     * ... and the class type.
     */

    memcpy(&conv, here, sizeof(unsigned short int));
    y->class = ntohs(conv);
    here += 2;

    /*
     * Question blocks stop here ...
     */

    if (question != 0) return (here);
    /*
     * ... while answer blocks carry a TTL and the actual data.
     */

    /* safe to read TTL and RDLENGTH? */
    if ((here+6) > limit){
	return (NULL);
    }
    memcpy(&y->ttl, here, sizeof(unsigned long int));
    y->ttl = ntohl(y->ttl);
    here += 4;
    /*
     * Fetch the resource data.
     */
   // memcpy(&y->len, here, sizeof(unsigned short int));
    memcpy(&check_len, here, sizeof(unsigned short));
    //len = y->len = ntohs(y->len);
    len = y->len = check_len;
    here += 2;
    /* safe to read RDATA? */
    if ((here + y->len) > limit){
    	 return (NULL);
    }
    if (y->len > sizeof(y->data) - 4) {
	y->len = sizeof(y->data) - 4;
    }

    memcpy(y->data, here, y->len);
    here += len;
    y->data[y->len] = 0;

    return (here);
}



extern int find_domain(char *data, int dlen, char *pattern, int plen, char term);
//Brad add for url bloacking ipaddress
extern char url_tbl[20][40];
extern int url_filter;
extern int total_entryURL;
extern unsigned char matchkye[10];
extern unsigned int ipaddr[50];
int check_urlfilter(char *type, unsigned char *packet, int len)
{
  int	i;
  rr_t	y;
  dnsheader_t *x;
  unsigned char *limit;
  struct in_addr ping_inIp1;
  int found=0;
  int match_index=0;
  int index_offset=0;
  int isKeywordMatched = 0;
  char dest_addr[30];
  char cmdBuf[300];
  unsigned int setIpaddr=0;
  unsigned char isSet=0;
  unsigned int byte1=0;
  unsigned int byte2=0;
  unsigned int byte3=0;
  unsigned int byte4=0;

if(url_filter ==0)
	return 0;
  if ((x = decode_header(packet, len)) == NULL ) {
    return (-1);
  }
  limit = x->packet + len;

  if (x->u & MASK_Z) log_debug("Z is set");

//  fprintf(stderr, "\n");
//  fprintf(stderr, "- -- %s\n", type);
//  raw_dump(x);
  
 // fprintf(stderr, "\n");
 // fprintf(stderr, "id= %u, q= %d, opc= %d, aa= %d, wr/ra= %d/%d, "
//	  "trunc= %d, rcode= %d [%04X]\n",
//	  x->id, GET_QR(x->u), GET_OPCODE(x->u), GET_AA(x->u),
//	  GET_RD(x->u), GET_RA(x->u), GET_TC(x->u), GET_RCODE(x->u), x->u);

 // fprintf(stderr, "qd= %u\n", x->qdcount);
  
  if ((x->here = read_record1(x, &y, x->here, 1, limit)) == NULL) {
    free_packet(x);
    return(-1);
  }
//check wheather to pick the ipaddress for iptables rules
	for(i=0; i< total_entryURL ;i++) {	   
		 found = find_domain(y.name,strlen(y.name), url_tbl[i], strlen(url_tbl[i]),'\0');	   
		if(found){
				//printf("Got the match case :%s <----->%s\n",y.name ,url_tbl[i]);
				isKeywordMatched = 1;
				match_index = i;
			      break;
			}
		}
	if(isKeywordMatched == 0){
		if(x!=NULL) //Free memory
			free_packet(x);
		
		return 0;
	}
  //fprintf(stderr, "  name= %s, type= %d, class= %d\n", y.name, y.type, y.class);
//  fprintf(stderr, "ans= %u\n", x->ancount);
  for (i = 0; i < x->ancount; i++) {
    if ((x->here = read_record1(x, &y, x->here, 0, limit)) == NULL) {
      free_packet(x);
      return(-1);
    }
    //check if the type ==1, and the class ==1 and the len ==4, then save the data
  //  fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n", y.name, y.type, y.class, y.ttl);
	if(y.type==1 && y.class==1&& y.len==4 && isKeywordMatched ==1){
		index_offset = match_index*5;
		byte1= (unsigned char)y.data[0];
		byte2= (unsigned char)y.data[1];
		byte3= (unsigned char)y.data[2];
		byte4= (unsigned char)y.data[3];
		setIpaddr = (( byte1<< 24) | (byte2 << 16)|(byte3 << 8)| byte4);
		for(i=0;i<5;i++){
			if(ipaddr[index_offset+i] == setIpaddr){
				isSet=1;
				break;
			}	    
  		}
		//if(matchkye[match_index] == 0){
		if(isSet==0){
			sprintf(dest_addr, "%d.%d.%d.%d", (unsigned char)y.data[0], (unsigned char)y.data[1], (unsigned char)y.data[2], (unsigned char)y.data[3]);
			sprintf(cmdBuf, "iptables -I FORWARD -i br0 -p icmp -d %s -j DROP", dest_addr);
			system(cmdBuf);
			//printf("the keyword %s index= %d has been set\n",url_tbl[match_index], match_index );
			for(i=0;i<5;i++){
				if(ipaddr[index_offset+i] ==0){
					ipaddr[index_offset+i]= setIpaddr;
					//printf("ipaddr index=%d, ip=%08X\n",index_offset+i, ipaddr[index_offset+i]); 
					break;
				}
			}	
		}
	}	    
  }
	    isKeywordMatched = 0;
	    
	    
  //fprintf(stderr, "ns= %u\n", x->nscount);
  #if 0
  for (i = 0; i < x->nscount; i++) {
    if ((x->here = read_record1(x, &y, x->here, 0, limit))==NULL) {
      free_packet(x);
      return(-1);
    }
    fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
	    y.name, y.type, y.class, y.ttl);
  }
    
  fprintf(stderr, "ar= %u\n", x->arcount);
  for (i = 0; i < x->arcount; i++) {
    if ((x->here = read_record(x, &y, x->here, 0, limit))==NULL) {
      free_packet(x);
      return(-1);
    }
    fprintf(stderr, "  name= %s, type= %d, class= %d, ttl= %lu\n",
	    y.name, y.type, y.class, y.ttl);
  }
	#endif    
	
	 
  //fprintf(stderr, "\n");
  
  free_packet(x);

  return (0);
}

#endif


