/* 
This module is an adaption of code from the tcpd-1.4 package written
by Wietse Venema, Eindhoven University of Technology, The Netherlands.

The code is used here with permission.

The code has been considerably changed from the original. Bug reports
should be sent to Andrew.Tridgell@anu.edu.au
*/

#include "includes.h"
#include "loadparm.h"

#ifndef	INADDR_NONE
#define	INADDR_NONE	(-1)		/* XXX should be 0xffffffff */
#endif


#define FROM_ADDRLEN  (4*3+3+1)
#define GOOD True
#define BAD False

#define CLIENT_MATCH client_match

/* Delimiters for lists of daemons or clients. */

static char sep[] = ", \t";

/* Constants to be used in assignments only, not in comparisons... */

#define	YES		1
#define	NO		0
#define	FAIL		(-1)

/* Forward declarations. */
BOOL allow_access(char *deny_list,char *allow_list,struct from_host *client);
static int list_match(char *list,char *item, int (*match_fn)());
static int client_match(char *tok,char *item);
static int string_match(char *tok,char *string);
static int masked_match(char *tok, char *slash, char *string);
static int matchname(char *remotehost,struct in_addr  addr);
BOOL fromhost(int sock,struct from_host *f);


/* Size of logical line buffer. */
#define	BUFLEN 2048


/* return true if access should be allowed to a service*/
BOOL check_access(int snum)
{
  extern int Client;
  extern struct from_host Client_info;
  char *denyl,*allowl;
  BOOL ret = False;

  denyl = lp_hostsdeny(snum);
  if (denyl) denyl = strdup(denyl);

  allowl = lp_hostsallow(snum);
  if (allowl) allowl = strdup(allowl);


  fromhost(Client,&Client_info);

  if ((!denyl || *denyl==0) && (!allowl || *allowl==0))
    ret = True;

  if (!ret)
    {
      if (!fromhost(Client,&Client_info))
	Debug(0,"ERROR: Can't get from_host info\n");
      else
	{
	  if (allow_access(denyl,allowl,&Client_info))
	    {
	      Debug(2,"Allowed connection from %s (%s) to %s\n",
		    Client_info.name,Client_info.addr,lp_servicename(snum));
	      ret = True;
	    }
	  else
	    Debug(0,"Denied connection from %s (%s) to %s\n",
		  Client_info.name,Client_info.addr,lp_servicename(snum));
	}
    }

  if (denyl) free(denyl);
  if (allowl) free(allowl);
  return(ret);
}


/* return true if access should be allowed */
BOOL allow_access(char *deny_list,char *allow_list,struct from_host *client)
{
  /* if theres no deny list and no allow list then allow access */
  if ((!deny_list || *deny_list == 0) && (!allow_list || *allow_list == 0))
    return(True);  

  /* if there is an allow list but no deny list then allow only hosts
     on the allow list */
  if (!deny_list || *deny_list == 0)
    return(list_match(allow_list,(char *)client,CLIENT_MATCH));

  /* if theres a deny list but no allow list then allow
     all hosts not on the deny list */
  if (!allow_list || *allow_list == 0)
    return(!list_match(deny_list,(char *)client,CLIENT_MATCH));

  /* if there are both type of list then allow all hosts on the allow list */
  if (list_match(allow_list,(char *)client,CLIENT_MATCH))
    return (True);

  /* if there are both type of list and it's not on the allow then
     allow it if its not on the deny */
  if (list_match(deny_list,(char *)client,CLIENT_MATCH))
    return (False);

  return (True);
}

/* list_match - match an item against a list of tokens with exceptions */
static int list_match(char *list,char *item, int (*match_fn)())
{
    char   *tok;
    int     match = NO;

    /*
     * Process tokens one at a time. We have exhausted all possible matches
     * when we reach an "EXCEPT" token or the end of the list. If we do find
     * a match, look for an "EXCEPT" list and recurse to determine whether
     * the match is affected by any exceptions.
     */

    for (tok = strtok(list, sep); tok != 0; tok = strtok((char *) 0, sep)) {
	if (strcasecmp(tok, "EXCEPT") == 0)	/* EXCEPT: give up */
	    break;
	if ((match = (*match_fn) (tok, item)))	/* YES or FAIL */
	    break;
    }
    /* Process exceptions to YES or FAIL matches. */

    if (match != NO) {
	while ((tok = strtok((char *) 0, sep)) && strcasecmp(tok, "EXCEPT"))
	     /* VOID */ ;
	if (tok == 0 || list_match((char *) 0, item, match_fn) == NO)
	    return (match);
    }
    return (NO);
}

/* client_match - match host name and address against token */
static int client_match(char *tok,char *item)
{
    struct from_host *client = (struct from_host *) item;
    int     match;

    /*
     * Try to match the address first. If that fails, try to match the host
     * name if available.
     */

    if ((match = string_match(tok, client->addr)) == 0)
	if (client->name[0] != 0)
	    match = string_match(tok, client->name);
    return (match);
}

/* string_match - match string against token */
static int string_match(char *tok,char *string)
{
    int     tok_len;
    int     str_len;
    char   *cut;
#ifdef	NETGROUP
    static char *mydomain = 0;
#endif

    /*
     * Return YES if a token has the magic value "ALL". Return FAIL if the
     * token is "FAIL". If the token starts with a "." (domain name), return
     * YES if it matches the last fields of the string. If the token has the
     * magic value "LOCAL", return YES if the string does not contain a "."
     * character. If the token ends on a "." (network number), return YES if
     * it matches the first fields of the string. If the token begins with a
     * "@" (netgroup name), return YES if the string is a (host) member of
     * the netgroup. Return YES if the token fully matches the string. If the
     * token is a netnumber/netmask pair, return YES if the address is a
     * member of the specified subnet.
     */

    if (tok[0] == '.') {			/* domain: match last fields */
	if ((str_len = strlen(string)) > (tok_len = strlen(tok))
	    && strcasecmp(tok, string + str_len - tok_len) == 0)
	    return (YES);
    } else if (tok[0] == '@') {			/* netgroup: look it up */
#ifdef	NETGROUP
	if (mydomain == 0)
	    yp_get_default_domain(&mydomain);
	if (!isdigit(string[0])
	    && innetgr(tok + 1, string, (char *) 0, mydomain))
	    return (YES);
#else
	Debug(0,"access: netgroup support is not configured");
	return (NO);
#endif
    } else if (strcasecmp(tok, "ALL") == 0) {	/* all: match any */
	return (YES);
    } else if (strcasecmp(tok, "FAIL") == 0) {	/* fail: match any */
	return (FAIL);
    } else if (strcasecmp(tok, "LOCAL") == 0) {	/* local: no dots */
	if (strchr(string, '.') == 0 && strcasecmp(string, "unknown") != 0)
	    return (YES);
    } else if (!strcasecmp(tok, string)) {	/* match host name or address */
	return (YES);
    } else if (tok[(tok_len = strlen(tok)) - 1] == '.') {	/* network */
	if (strncmp(tok, string, tok_len) == 0)
	    return (YES);
    } else if ((cut = strchr(tok, '/')) != 0) {	/* netnumber/netmask */
	if (isdigit(string[0]) && masked_match(tok, cut, string))
	    return (YES);
    }
    return (NO);
}

/* masked_match - match address against netnumber/netmask */
static int masked_match(char *tok, char *slash, char *string)
{
    unsigned long net;
    unsigned long mask;
    unsigned long addr;

    if ((addr = inet_addr(string)) == INADDR_NONE)
	return (NO);
    *slash = 0;
    net = inet_addr(tok);
    *slash = '/';
    if (net == INADDR_NONE || (mask = inet_addr(slash + 1)) == INADDR_NONE) {
	Debug(0,"access: bad net/mask access control: %s", tok);
	return (NO);
    }
    return ((addr & mask) == net);
}


/* fromhost - find out what is at the other end of a socket */
BOOL fromhost(int sock,struct from_host *f)
{
    static struct sockaddr sa;
    struct sockaddr_in *sin = (struct sockaddr_in *) (&sa);
    struct hostent *hp;
    int     length = sizeof(sa);
    static char addr_buf[FROM_ADDRLEN];
    static char name_buf[MAXHOSTNAMELEN];

    if (getpeername(sock, &sa, &length) < 0) 
      {
	Debug(0,"getpeername failed\n");
	return(False);
      }

    f->sin = sin;
    f->addr = strcpy(addr_buf,(char *)inet_ntoa(sin->sin_addr));

    /* Look up the remote host name. */
    if ((hp = gethostbyaddr((char *) &sin->sin_addr,
			    sizeof(sin->sin_addr),
			    AF_INET)) == 0) {
      Debug(0,"Gethostbyaddr failed\n");
      return(False);
    }

    /* Save the host name. A later gethostbyxxx() call may clobber it. */
    f->name = strncpy(name_buf, hp->h_name, sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = 0;

    /*
     * Verify that the host name does not belong to someone else. If host
     * name verification fails, pretend that the host name lookup failed.
     */
    if (!matchname(f->name, sin->sin_addr))
      {
	Debug(0,"Matchname failed\n");
	return(False);
      }

    return(True);	
}

/* matchname - determine if host name matches IP address */
static int matchname(char *remotehost,struct in_addr  addr)
{
  struct hostent *hp;
  int     i;
  
  if ((hp = gethostbyname(remotehost)) == 0) {
    Debug(0,"gethostbyname(%s): lookup failure", remotehost);
    return (BAD);
  } 

    /*
     * Make sure that gethostbyname() returns the "correct" host name.
     * Unfortunately, gethostbyname("localhost") sometimes yields
     * "localhost.domain". Since the latter host name comes from the
     * local DNS, we just have to trust it (all bets are off if the local
     * DNS is perverted). We always check the address list, though.
     */
  
  if (strcasecmp(remotehost, hp->h_name)
      && strcasecmp(remotehost, "localhost")) {
    Debug(0,"host name/name mismatch: %s != %s",
	  remotehost, hp->h_name);
    return (BAD);
  }
	
  /* Look up the host address in the address list we just got. */
  for (i = 0; hp->h_addr_list[i]; i++) {
    if (memcmp(hp->h_addr_list[i], (caddr_t) & addr, sizeof(addr)) == 0)
      return (GOOD);
  }

  /*
   * The host name does not map to the original host address. Perhaps
   * someone has compromised a name server. More likely someone botched
   * it, but that could be dangerous, too.
   */
  
  Debug(0,"host name/address mismatch: %s != %s",
	inet_ntoa(addr), hp->h_name);
  return (BAD);
}


