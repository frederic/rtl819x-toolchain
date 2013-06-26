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

pstring cur_dir = "\\";
pstring service;
pstring desthost;
pstring myname = "";
pstring password = "";
BOOL got_pass = False;
BOOL connect_as_printer = False;

pstring debugf = DEBUGFILE;

pstring tarfilename = "";
FILE *tarfile = NULL;
#define DOTAR (*tarfilename != 0)

char *InBuffer = NULL;
char *OutBuffer = NULL;
int cnum = 0;
int pid = 0;
int gid = 0;
int uid = 0;
int mid = 0;
int myumask = 0755;

int max_xmit = BUFFER_SIZE;

extern BOOL NeedSwap;

BOOL prompt = True;

int printmode = 1;

BOOL recurse = False;
BOOL lowercase = False;

BOOL have_ip = False;

struct in_addr dest_ip;

#define SEPARATORS " \t\n\r"

BOOL abort_mget = True;

int Protocol = PROT_CORE;

BOOL readbraw_supported = False;
BOOL writebraw_supported = False;

int tarbuffersize = 0;
char *tarbuffer = NULL;

/****************************************************************************
change directory
****************************************************************************/
void cmd_cd(char *inbuf,char *outbuf )
{
  char *p;
  pstring saved_dir;
  int errcls, errcode;

  p = strtok(NULL,SEPARATORS);
  if (p)
    {
      /* Save the current directory in case the
	 new directory is invalid */
      strcpy(saved_dir, cur_dir);
      if (*p == '\\')
	strcpy(cur_dir,p);
      else
	strcat(cur_dir,p);
      clean_name(cur_dir);
      memset(outbuf,0,smb_size);
      set_message(outbuf,0,4 + strlen(cur_dir));
      CVAL(outbuf,smb_com) = SMBchkpth;
      SSVAL(outbuf,smb_tid,cnum);
      SSVAL(outbuf,smb_pid,pid);
      SSVAL(outbuf,smb_uid,uid);
      SSVAL(outbuf,smb_mid,mid);
      p = smb_buf(outbuf);
      *p++ = 4;
      strcpy(p,cur_dir);
      strcat(cur_dir,"\\");
      clean_name(cur_dir);

      if (!strequal(cur_dir,"\\"))
	{
	  send_smb(outbuf);
	  receive_smb(inbuf);
	  /* Check if this was valid */
	  if ((errcls = CVAL(inbuf,smb_rcls) != 0))
	    {
	      errcode=SVAL(inbuf,smb_err);
	      Debug(0,"cd %s : ", cur_dir);
	      if(errcls == ERRDOS)
		{
		  if(errcode == eACCESS_DENIED)
		    Debug(0,"permision denied\n");
		  else if ( errcode == eINVALID_PATH)
		    Debug(0,"no such directory\n");
		  else
		    Debug(0,"Error %d\n", errcode);
		}
	      else
		Debug(0,"Error %d\n",errcode);
	      strcpy(cur_dir,saved_dir);
	    }
	}
    }
  else
    Debug(0,"Current directory is %s\n",cur_dir);
}

/****************************************************************************
do a directory listing, calling fn on each file found
****************************************************************************/
void do_dir(char *inbuf,char *outbuf,char *mask,int attribute,void (*fn)())
{
  char *p;
  int received = 0;
  BOOL first = True;
  char status[21];
  int num_asked = (max_xmit - 100)/DIR_STRUCT_SIZE;
  int num_received = 0;

  while (1)
    {
      memset(outbuf,0,smb_size);
      if (first)	
	set_message(outbuf,2,5 + strlen(mask));
      else
	set_message(outbuf,2,5 + 21);

      CVAL(outbuf,smb_com) = SMBsearch;
      SSVAL(outbuf,smb_tid,cnum);
      SSVAL(outbuf,smb_pid,pid);
      SSVAL(outbuf,smb_uid,uid);
      SSVAL(outbuf,smb_mid,mid);
      SSVAL(outbuf,smb_vwv0,num_asked);
      SSVAL(outbuf,smb_vwv1,attribute);
  
      p = smb_buf(outbuf);
      *p++ = 4;
      
      if (first)
	strcpy(p,mask);
      else
	strcpy(p,"");
      p += strlen(p) + 1;
      
      *p++ = 5;
      if (first)
	SSVAL(p,0,0);
      else
	{
	  SSVAL(p,0,21);
	  p += 2;
	  memcpy(p,status,21);
	}

      send_smb(outbuf);
      receive_smb(inbuf);
      first = False;

#if 0
      if (CVAL(inbuf,smb_rcls) != 0)
	break;
#endif
      
      num_received = received = SVAL(inbuf,smb_vwv0);
      if (received == 0)
	break;

      p = smb_buf(inbuf) + 3;

      while (received--)
	{
	  uint32 size;
	  uint32 Date;
	  time_t Time;
	  int attr;
	  char *name;
	  char attrstr[10]="";

	  memcpy(status,p,21);

	  attr = CVAL(p,21);
	  memcpy(&Date,p+22,4);
	  Time = make_unix_date(Date);
	  size = SVAL(p,26) + (SVAL(p,28)<<16);
	  name = p+30;

	  if (attr & aDIR) strcat(attrstr,"D");
	  if (attr & aARCH) strcat(attrstr,"A");
	  if (attr & aHIDDEN) strcat(attrstr,"H");
	  if (attr & aSYSTEM) strcat(attrstr,"S");
	  if (attr & aRONLY) strcat(attrstr,"R");

	  if (fn)
	    fn(name,attr,size,Time);
	  else
	    Debug(0,"%20.20s%7.7s%7d  %s",name,attrstr,size,asctime(localtime(&Time)));
	  p += DIR_STRUCT_SIZE;
	}
    }
}

/****************************************************************************
get a directory listing
****************************************************************************/
void cmd_dir(char *inbuf,char *outbuf)
{
  int attribute = aDIR | aSYSTEM | aHIDDEN;
  pstring mask;
  char *p;

  strcpy(mask,cur_dir);
  if(mask[strlen(mask)-1]!='\\')
    strcat(mask,"\\");

  p = strtok(NULL,SEPARATORS);
  if (p)
    {
      if (*p == '\\')
	strcpy(mask,p);
      else
	strcat(mask,p);
    }
  else
    strcat(mask,"*.*");

  do_dir(inbuf,outbuf,mask,attribute,NULL);
}


/****************************************************************************
start a tar file if it isn't already open
****************************************************************************/
BOOL starttar(void)
{
  if (tarfile) return(True);

  if (strequal(tarfilename,"-"))
    tarfile = stdout;
  else
    tarfile = fopen(tarfilename,"w");

  if (!tarfile)
    {
      Debug(0,"Couldn't open tarfile %s\n",tarfilename);
      abort_mget = True;
      return False;
    }

  if (tarbuffersize > 0)
    {
      tarbuffer = (char *)malloc(tarbuffersize);
      if (tarbuffer)
	setbuffer(tarfile,tarbuffer,tarbuffersize);
    }  

  return True;
}


/****************************************************************************
write a tar header
****************************************************************************/
BOOL maketarhead(char *name,int dosmode,int size,time_t mtime)
{
  union tarhblock tarhead;      
  pstring unixname;
  mode_t result = 0444;
  
  strcpy(unixname,name);
  unix_format(unixname);
  /* trim off any ./ then add it back */
  trim_string(unixname,"./",NULL);

  {
    pstring tmpnam;
    strcpy(tmpnam,unixname);
    strcpy(unixname,"./");
    strcat(unixname,tmpnam);
  }

  if (strlen(unixname) >= (NAMSIZ-1))
    {
      Debug(0,"Name too long for tar format!\n");
      unixname[NAMSIZ-2]=0;
    }

  memset(&tarhead,0,sizeof(tarhead));
  strncpy(tarhead.dbuf.name,unixname,NAMSIZ);
  
  
  if ((dosmode & aRONLY) == 0)
    result |= (S_IWUSR | S_IWGRP | S_IWOTH);
  if ((dosmode & aDIR) != 0)
    result |= (S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH);
  if ((dosmode & aARCH) != 0)
    result |= S_IXUSR;
  
  if ((dosmode & aSYSTEM) != 0)
    result |= S_IXGRP;
  
  if ((dosmode & aHIDDEN) != 0)
    result |= S_IXOTH;  
  
  result &= ((~myumask) | S_IFDIR);
  
  sprintf(tarhead.dbuf.mode,"%06o ",result);
  sprintf(tarhead.dbuf.uid,"%06o ",uid);
  sprintf(tarhead.dbuf.gid,"%06o ",gid);
  sprintf(tarhead.dbuf.size,"%011o ",size);
  sprintf(tarhead.dbuf.mtime,"%011o ",mtime);
  memset(tarhead.dbuf.chksum,' ',8);
  sprintf(tarhead.dbuf.chksum,"%06o ",
	  byte_checksum((unsigned char *)&tarhead,sizeof(tarhead)));
  
  if (fwrite(&tarhead,sizeof(tarhead),1,tarfile) != 1)
    {
      Debug(0,"Error writing to tarfile\n");
      abort_mget = True;
      return(False);
    }      
  return(True);
}


/****************************************************************************
get a file from rname to lname
****************************************************************************/
void do_get(char *rname,char *lname)
{
  int handle,fnum;
  uint32 size,nread=0;
  char *p;
  BOOL newhandle = False;
  unsigned char *inbuf,*outbuf;
  int dosmode;
  time_t mtime;

  if (lowercase)
    strlower(lname);

  if (DOTAR && !starttar())
    return;

  inbuf = (unsigned char *)malloc(BUFFER_SIZE);
  outbuf = (unsigned char *)malloc(BUFFER_SIZE);

  if (!inbuf || !outbuf)
    {
      Debug(0,"out of memory\n");
      return;
    }

  memset(outbuf,0,smb_size);
  set_message(outbuf,2,2 + strlen(rname));

  CVAL(outbuf,smb_com) = SMBopen;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,0);
  SSVAL(outbuf,smb_vwv1,0);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,rname);
  clean_name(rname);

  if (!tarfile)
    {
      if(!strcmp(lname,"-"))
	handle = fileno(stdout);
      else 
	{
	  handle = creat(lname,0644);
	  newhandle = True;
	}
      if (handle < 0)
	{
	  Debug(0,"Error opening local file %s\n",lname);
	  free(inbuf);free(outbuf);
	  return;
	}
    }

  send_smb(outbuf);
  receive_smb(inbuf);

  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d opening remote file %s\n",SVAL(inbuf,smb_err),rname);
      if(newhandle)
	close(handle);
      free(inbuf);free(outbuf);
      return;
    }

  fnum = SVAL(inbuf,smb_vwv0);
  size = SVAL(inbuf,smb_vwv4) + (SVAL(inbuf,smb_vwv5)<<16);
  dosmode = SVAL(inbuf,smb_vwv1);
  mtime = SVAL(inbuf,smb_vwv2) | (SVAL(inbuf,smb_vwv3) << 16);

  if (tarfile)
      Debug(2,"adding file %s of size %d bytes to %s\n",rname,size,tarfilename);
  else
      Debug(2,"getting file %s of size %d bytes as %s\n",rname,size,lname);

  if (tarfile && !maketarhead(rname,dosmode,size,mtime))
    nread = size;


  while (nread < size)
    {
      unsigned char *dataptr;
      int datalen;

      if (readbraw_supported)
	{
	  extern int Client;
	  memset(outbuf,0,smb_size);
	  set_message(outbuf,8,0);
	  CVAL(outbuf,smb_com) = SMBreadbraw;
	  SSVAL(outbuf,smb_tid,cnum);
	  SSVAL(outbuf,smb_pid,pid);
	  SSVAL(outbuf,smb_uid,uid);
	  SSVAL(outbuf,smb_mid,mid);
	  SSVAL(outbuf,smb_vwv0,fnum);
	  SIVAL(outbuf,smb_vwv1,nread);
	  SSVAL(outbuf,smb_vwv3,MIN(size-nread,BUFFER_SIZE-4));
	  SSVAL(outbuf,smb_vwv4,0);
	  SIVAL(outbuf,smb_vwv5,1000);
	  send_smb(outbuf);

	  /* Now read the raw data into the buffer and write it */	  
	  if(!read_data(Client,inbuf,4)) {
	    Debug(0,"Failed to read length in readbraw\n");	    
	    exit(0);
	  }

	  /* Even though this is not an smb message, smb_len
	     returns the generic length of an smb message */
	  datalen = smb_len(inbuf);
	  if(!read_data(Client,inbuf,datalen)) {
	    Debug(0,"Failed to read data in readbraw\n");
	    exit(1);
	  }

	  dataptr = inbuf;
	  
	}
      else
	{
	  memset(outbuf,0,smb_size);
	  set_message(outbuf,5,0);
	  CVAL(outbuf,smb_com) = SMBread;
	  SSVAL(outbuf,smb_tid,cnum);
	  SSVAL(outbuf,smb_pid,pid);
	  SSVAL(outbuf,smb_uid,uid);
	  SSVAL(outbuf,smb_mid,mid);
	  SSVAL(outbuf,smb_vwv0,fnum);
	  SSVAL(outbuf,smb_vwv1,MIN(1024*((max_xmit-1024)/1024),size - nread));
	  SSVAL(outbuf,smb_vwv2,nread & 0xFFFF);
	  SSVAL(outbuf,smb_vwv3,nread >> 16);
	  SSVAL(outbuf,smb_vwv4,size - nread);

	  send_smb(outbuf);
	  receive_smb(inbuf);

	  if (CVAL(inbuf,smb_rcls) != 0)
	    {
	      Debug(0,"Error %d reading remote file\n",SVAL(inbuf,smb_err));
	      break;
	    }

	  datalen = SVAL(inbuf,smb_vwv0);
	  dataptr = smb_buf(inbuf) + 3;
	}
 
      if (tarfile)
	{
	  if (fwrite(dataptr,1,datalen,tarfile) != datalen)
	    {
	      Debug(0,"Error writing to tarfile\n");
	      abort_mget = True;
	      break;
	    }      	  
	}
      else
	if (write(handle,dataptr,datalen) != datalen)
	  {
	    Debug(0,"Error writing local file\n");
	    break;
	  }
      
      nread += datalen;
    }

  if (tarfile)
    {
      int remainder = (TBLOCK - (nread%TBLOCK))%TBLOCK;
      if (remainder > 0)
	{
	  memset(outbuf,0,remainder);
	  if (fwrite(outbuf,1,remainder,tarfile) != 
	      remainder)
	    {
	      Debug(0,"Error writing to tarfile\n");
	      abort_mget = True;
	    }      
	}
    }	  

  memset(outbuf,0,smb_size);
  set_message(outbuf,3,0);
  CVAL(outbuf,smb_com) = SMBclose;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,fnum);
  SSVAL(outbuf,smb_vwv1,0);
  SSVAL(outbuf,smb_vwv2,0);

  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d closing remote file\n",SVAL(inbuf,smb_err));
      if(newhandle)
	close(handle);
      free(inbuf);free(outbuf);
      return;
    }

  if(newhandle)
    close(handle);
  free(inbuf);free(outbuf);
}


/****************************************************************************
get a file
****************************************************************************/
void cmd_get(void)
{
  pstring lname;
  pstring rname;
  char *p;

  strcpy(rname,cur_dir);
  strcat(rname,"\\");

  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"get <filename>\n");
      return;
    }
  strcat(rname,p); 
  clean_name(rname);
  strcpy(lname,p);

  p = strtok(NULL,SEPARATORS);
  if (p)
    strcpy(lname,p);      

  do_get(rname,lname);
}


/****************************************************************************
do a mget operation on one file
****************************************************************************/
void do_mget(char *name,int attr,int size,time_t mtime)
{
  pstring rname;
  pstring quest;

  if (strequal(name,".") || strequal(name,".."))
    return;

  if (abort_mget)
    {
      Debug(0,"mget aborted\n");
      return;
    }

  if (attr & aDIR)
    sprintf(quest,"Get directory %s? ",name);
  else
    sprintf(quest,"Get file %s? ",name);

  if (prompt && !yesno(quest)) return;

  if (attr & aDIR)
    {
      pstring saved_curdir;
      pstring mget_mask;
      unsigned char *inbuf,*outbuf;

      inbuf = (unsigned char *)malloc(BUFFER_SIZE);
      outbuf = (unsigned char *)malloc(BUFFER_SIZE);

      if (!inbuf || !outbuf)
	{
	  Debug(0,"out of memory\n");
	  return;
	}

      strcpy(saved_curdir,cur_dir);

      strcat(cur_dir,name);
      strcat(cur_dir,"\\");

      unix_format(name);
      if (!DOTAR)
	{
	  if (!directory_exist(name) && mkdir(name,0777) != 0) 
	    {
	      Debug(0,"failed to create directory %s\n",name);
	      strcpy(cur_dir,saved_curdir);
	      free(inbuf);free(outbuf);
	      return;
	    }

	  if (chdir(name) != 0)
	    {
	      Debug(0,"failed to chdir to directory %s\n",name);
	      strcpy(cur_dir,saved_curdir);
	      free(inbuf);free(outbuf);
	      return;
	    }
	}       
      else
	{
	  if (!starttar() || !maketarhead(cur_dir,attr,0,mtime))
	    return;
	}

      strcpy(mget_mask,cur_dir);
      strcat(mget_mask,"*.*");
      
      do_dir((char *)inbuf,(char *)outbuf,
	     mget_mask,aSYSTEM | aHIDDEN | aDIR,do_mget);
      if (!DOTAR)
	chdir("..");
      strcpy(cur_dir,saved_curdir);
      free(inbuf);free(outbuf);
    }
  else
    {
      strcpy(rname,cur_dir);
      strcat(rname,name);

      do_get(rname,name);
    }
}


/****************************************************************************
do a mget command
****************************************************************************/
void cmd_mget(char *inbuf,char *outbuf)
{
  int attribute = aSYSTEM | aHIDDEN;
  pstring mget_mask="";
  char *p;

  if (recurse)
    attribute |= aDIR;

  abort_mget = False;

  while ((p = strtok(NULL,SEPARATORS)))
    {
      strcpy(mget_mask,cur_dir);
      if(mget_mask[strlen(mget_mask)-1]!='\\')
	strcat(mget_mask,"\\");

      if (*p == '\\')
	strcpy(mget_mask,p);
      else
	strcat(mget_mask,p);
    }

  do_dir((char *)inbuf,(char *)outbuf,mget_mask,attribute,do_mget);
}

/****************************************************************************
make a directory of name "name"
****************************************************************************/
BOOL do_mkdir(char *name)
{
  char *p;
  unsigned char *inbuf,*outbuf;

  inbuf = (unsigned char *)malloc(BUFFER_SIZE);
  outbuf = (unsigned char *)malloc(BUFFER_SIZE);

  if (!inbuf || !outbuf)
    {
      Debug(0,"out of memory\n");
      return False;
    }

  memset(outbuf,0,smb_size);
  set_message(outbuf,0,2 + strlen(name));
  
  CVAL(outbuf,smb_com) = SMBmkdir;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,name);
  
  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d making remote directory file %s\n",SVAL(inbuf,smb_err),name);
      free(inbuf);free(outbuf);
      return(False);
    }

  free(inbuf);free(outbuf);
  return(True);
}

/****************************************************************************
make a directory
****************************************************************************/
void cmd_mkdir(void)
{
  pstring mask;
  char *p;
  
  strcpy(mask,cur_dir);
  strcat(mask,"\\");
  
  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"mkdir <dirname>\n");
      return;
    }
  strcat(mask,p);

  do_mkdir(mask);
}

/****************************************************************************
put a single file
****************************************************************************/
void do_put(char *rname,char *lname)
{
  int handle,fnum;
  uint32 size,nread=0;
  char *p;
  unsigned char *inbuf,*outbuf;

  inbuf = (unsigned char *)malloc(BUFFER_SIZE);
  outbuf = (unsigned char *)malloc(BUFFER_SIZE);

  if (!inbuf || !outbuf)
    {
      Debug(0,"out of memory\n");
      return;
    }

  
  memset(outbuf,0,smb_size);
  set_message(outbuf,3,2 + strlen(rname));
  
  CVAL(outbuf,smb_com) = SMBcreate;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,0);
  SSVAL(outbuf,smb_vwv1,0);
  SSVAL(outbuf,smb_vwv2,0);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,rname);
  
  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d opening remote file %s\n",SVAL(inbuf,smb_err),rname);
      free(inbuf);free(outbuf);
      return;
    }
  
  handle = open(lname,O_RDONLY);
  if (handle < 0)
    {
      Debug(0,"Error opening local file %s\n",lname);
      free(inbuf);free(outbuf);
      return;
    }

  
  fnum = SVAL(inbuf,smb_vwv0);
  size = file_size(lname);
  
  Debug(0,"putting file %s of size %d bytes as %s\n",lname,size,rname);
  
  while (nread < size)
    {
      int n = MIN(1024*((max_xmit-1024)/1024),size - nread);
  
      memset(outbuf,0,smb_size);
      set_message(outbuf,5,n + 3);
      CVAL(outbuf,smb_com) = SMBwrite;
      SSVAL(outbuf,smb_tid,cnum);
      SSVAL(outbuf,smb_pid,pid);
      SSVAL(outbuf,smb_uid,uid);
      SSVAL(outbuf,smb_mid,mid);
      SSVAL(outbuf,smb_vwv0,fnum);
      SSVAL(outbuf,smb_vwv1,n);
      SSVAL(outbuf,smb_vwv2,nread & 0xFFFF);
      SSVAL(outbuf,smb_vwv3,nread >> 16);
      SSVAL(outbuf,smb_vwv4,size - nread);
      CVAL(smb_buf(outbuf),0) = 1;
      SSVAL(smb_buf(outbuf),1,n);

      if (read(handle,smb_buf(outbuf)+3,n) != n)
	{
	  Debug(0,"Error reading local file\n");
	  break;
	}	  

      send_smb(outbuf);
      receive_smb(inbuf);

      if (CVAL(inbuf,smb_rcls) != 0)
	{
	  Debug(0,"Error %d writing remote file\n",SVAL(inbuf,smb_err));
	  break;
	}

      
      if (n != SVAL(inbuf,smb_vwv0))
	{
	  Debug(0,"Error: only wrote %d bytes\n",nread + SVAL(inbuf,smb_vwv0));
	  break;
	}

      nread += n;
    }

  memset(outbuf,0,smb_size);
  set_message(outbuf,3,0);
  CVAL(outbuf,smb_com) = SMBclose;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,fnum);
  SSVAL(outbuf,smb_vwv1,0);
  SSVAL(outbuf,smb_vwv2,0);

  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d closing remote file\n",SVAL(inbuf,smb_err));
      close(handle);
      free(inbuf);free(outbuf);
      return;
    }

  close(handle);
  free(inbuf);free(outbuf);
}

/****************************************************************************
put a file
****************************************************************************/
void cmd_put(void)
{
  pstring lname;
  pstring rname;
  char *p;
  
  strcpy(rname,cur_dir);
  strcat(rname,"\\");
  
  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"put <filename>\n");
      return;
    }
  strcpy(lname,p);
  
  p = strtok(NULL,SEPARATORS);
  if (p)
    strcat(rname,p);      
  else
    strcat(rname,lname);

  clean_name(rname);

  do_put(rname,lname);
}

/****************************************************************************
seek in a directory/file list until you get something that doesn't start with
the specified name
****************************************************************************/
BOOL seek_list(FILE *f,char *name)
{
  pstring s;
  while (!feof(f))
    {
      if (fscanf(f,"%s",s) != 1) return(False);
      trim_string(s,"./",NULL);
      if (strncmp(s,name,strlen(name)) != 0)
	{
	  strcpy(name,s);
	  return(True);
	}
    }
      
  return(False);
}

/****************************************************************************
mput some files
****************************************************************************/
void cmd_mput(void)
{
  pstring lname;
  pstring rname;

  char *p;
  
  while ((p = strtok(NULL,SEPARATORS)))
    {
      pstring cmd;
      pstring tmpnam;
      FILE *f;
      
      sprintf(tmpnam,"/tmp/ls.smb.%d",getpid());
      if (recurse)
	sprintf(cmd,"find . -name \"%s\" -print > %s",p,tmpnam);
      else
	sprintf(cmd,"/bin/ls %s > %s",p,tmpnam);
      system(cmd);

      f = fopen(tmpnam,"r");
      if (!f) continue;

      while (!feof(f))
	{
	  pstring quest;

	  if (fscanf(f,"%s",lname) != 1) break;
	  trim_string(lname,"./",NULL);

	again:

	  /* check if it's a directory */
	  if (directory_exist(lname))
	    {
	      if (!recurse) continue;
	      sprintf(quest,"Put directory %s? ",lname);
	      if (prompt && !yesno(quest)) 
		{
		  strcat(lname,"/");
		  if (!seek_list(f,lname))
		    break;
		  goto again;		    
		}
	      
	      strcpy(rname,cur_dir);
	      strcat(rname,lname);
	      if (!do_mkdir(rname))
		{
		  strcat(lname,"/");
		  if (!seek_list(f,lname))
		    break;
		  goto again;		    		  
		}

	      continue;
	    }
	  else
	    {
	      sprintf(quest,"Put file %s? ",lname);
	      if (prompt && !yesno(quest)) continue;

	      strcpy(rname,cur_dir);
	      strcat(rname,lname);
	    }
	  dos_format(rname);
	  do_put(rname,lname);
	}
      fclose(f);
      unlink(tmpnam);
    }
}


/****************************************************************************
print a file
****************************************************************************/
void cmd_print(char *inbuf,char *outbuf )
{
  int fnum;
  FILE *f = NULL;
  uint32 nread=0;
  pstring lname;
  pstring rname;
  char *p;

  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"print <filename>\n");
      return;
    }
  strcpy(lname,p);

  strcpy(rname,lname);
  p = strrchr(rname,'/');
  if (p)
    {
      pstring tname;
      strcpy(tname,p+1);
      strcpy(rname,tname);
    }

  if (strlen(rname) > 14)
    rname[14] = 0;

  if (strequal(lname,"-"))
    {
      f = stdin;
      strcpy(rname,"stdin");
    }
  
  clean_name(rname);

  memset(outbuf,0,smb_size);
  set_message(outbuf,2,2 + strlen(rname));
  
  CVAL(outbuf,smb_com) = SMBsplopen;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,0);
  SSVAL(outbuf,smb_vwv1,printmode);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,rname);
  
  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d opening printer for %s\n",SVAL(inbuf,smb_err),rname);
      return;
    }
  
  if (!f)
    f = fopen(lname,"r");
  if (!f)
    {
      Debug(0,"Error opening local file %s\n",lname);
      return;
    }

  
  fnum = SVAL(inbuf,smb_vwv0);
  
  Debug(1,"printing file %s as %s\n",lname,rname);
  
  while (!feof(f))
    {
      int n;
  
      memset(outbuf,0,smb_size);
      set_message(outbuf,1,3);

      /* for some strange reason the OS/2 print server can't handle large
	 packets when printing. weird */
      n = MIN(1024,max_xmit-(smb_len(outbuf)+4));

#if 0
      if (first)
	{
	  n = 0;
	  first = False;
	}
      else
#endif
	{
	  n = fread(smb_buf(outbuf)+3,1,n,f);
	  if (n <= 0) 
	    {
	      Debug(0,"read gave %d\n",n);
	      break;
	    }
	}

      smb_setlen(outbuf,smb_len(outbuf) + n);

      CVAL(outbuf,smb_com) = SMBsplwr;
      SSVAL(outbuf,smb_tid,cnum);
      SSVAL(outbuf,smb_pid,pid);
      SSVAL(outbuf,smb_uid,uid);
      SSVAL(outbuf,smb_mid,mid);
      SSVAL(outbuf,smb_vwv0,fnum);
      SSVAL(outbuf,smb_vwv1,n+3);
      CVAL(smb_buf(outbuf),0) = 1;
      SSVAL(smb_buf(outbuf),1,n);

      send_smb(outbuf);
      receive_smb(inbuf);

      if (CVAL(inbuf,smb_rcls) != 0)
	{
	  Debug(0,"Error %d printing remote file\n",SVAL(inbuf,smb_err));
	  break;
	}

      nread += n;
    }

  Debug(2,"%d bytes printed\n",nread);

  memset(outbuf,0,smb_size);
  set_message(outbuf,1,0);
  CVAL(outbuf,smb_com) = SMBsplclose;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,fnum);

  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d closing print file\n",SVAL(inbuf,smb_err));
      if (f != stdin)
	fclose(f);
      return;
    }

  if (f != stdin)
    fclose(f);
}


/****************************************************************************
delete some files
****************************************************************************/
void cmd_del(char *inbuf,char *outbuf )
{
  pstring mask;
  char *p;
  
  strcpy(mask,cur_dir);
  strcat(mask,"\\");
  
  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"del <filename>\n");
      return;
    }
  strcat(mask,p);


  memset(outbuf,0,smb_size);
  set_message(outbuf,1,2 + strlen(mask));
  
  CVAL(outbuf,smb_com) = SMBunlink;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  SSVAL(outbuf,smb_vwv0,0);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,mask);
  
  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d deleting remote file %s\n",SVAL(inbuf,smb_err),mask);
      return;
    }
  
}


/****************************************************************************
remove a directory
****************************************************************************/
void cmd_rmdir(char *inbuf,char *outbuf )
{
  pstring mask;
  char *p;
  
  strcpy(mask,cur_dir);
  strcat(mask,"\\");
  
  p = strtok(NULL,SEPARATORS);
  if (!p)
    {
      Debug(0,"rmdir <dirname>\n");
      return;
    }
  strcat(mask,p);


  memset(outbuf,0,smb_size);
  set_message(outbuf,0,2 + strlen(mask));
  
  CVAL(outbuf,smb_com) = SMBrmdir;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  
  p = smb_buf(outbuf);
  *p++ = 4;      
  strcpy(p,mask);
  
  send_smb(outbuf);
  receive_smb(inbuf);
  
  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"Error %d removing remote directory file %s\n",SVAL(inbuf,smb_err),mask);
      return;
    }
  
}

/****************************************************************************
toggle the prompt flag
****************************************************************************/
void cmd_prompt(void)
{
  prompt = !prompt;
  Debug(2,"prompting is now %s\n",prompt?"on":"off");
}

/****************************************************************************
toggle the lowercaseflag
****************************************************************************/
void cmd_lowercase(void)
{
  lowercase = !lowercase;
  Debug(2,"filename lowercasing is now %s\n",lowercase?"on":"off");
}


/****************************************************************************
set the tarfile name
****************************************************************************/
void cmd_tar(void)
{
  char *p = strtok(NULL,SEPARATORS);
  if (p)
    strcpy(tarfilename,p);

  Debug(2,"The tarfile is set to %s\n",tarfilename);
}


/****************************************************************************
close the tarfile and reset the tarfilename
****************************************************************************/
void cmd_endtar(void)
{
  if (tarfile)
    {
      char buf[TBLOCK*2];
      memset(buf,0,TBLOCK*2);
      if (fwrite(buf,TBLOCK,2,tarfile) != 2)
	Debug(0,"Error writing to tarfile\n");
      if (tarfile != stdout)
	fclose(tarfile);
      tarfile = NULL;
      if (tarbuffer)
	{
	  free(tarbuffer);
	  tarbuffer = NULL;
	}
    }
  strcpy(tarfilename,"");
}

/****************************************************************************
toggle the recurse flag
****************************************************************************/
void cmd_recurse(void)
{
  recurse = !recurse;
  Debug(2,"directory recursion is now %s\n",recurse?"on":"off");
}


/****************************************************************************
do a printmode command
****************************************************************************/
void cmd_printmode(void)
{
  char *p;
  pstring mode;

  p = strtok(NULL,SEPARATORS);
  if (p)
    {
      if (strequal(p,"text"))
	printmode = 0;      
      else
	{
	  if (strequal(p,"graphics"))
	    printmode = 1;
	  else
	    printmode = atoi(p);
	}
    }

  switch(printmode)
    {
    case 0: 
      strcpy(mode,"text");
      break;
    case 1: 
      strcpy(mode,"graphics");
      break;
    default: 
      sprintf(mode,"%d",printmode);
      break;
    }

  Debug(2,"the printmode is now %s\n",mode);
}

/****************************************************************************
do the lcd command
****************************************************************************/
void cmd_lcd(void)
{
  char *p;
  pstring d;

  p = strtok(NULL,SEPARATORS);
  if (p)
    chdir(p);
  Debug(2,"the local directory is now %s\n",GetWd(d));
}


/****************************************************************************
send a login command
****************************************************************************/
BOOL send_login(char *inbuf,char *outbuf )
{
  struct {
    int prot;
    char *name;
  }
  prots[] = 
    {
      {PROT_CORE,"PC NETWORK PROGRAM 1.0"},
      {PROT_COREPLUS,"MICROSOFT NETWORKS 1.03"},
      {-1,NULL}
    };
  char *pass = NULL;  
  pstring dev = "A:";
  char *p;
  int len = 4;
  int numprots;

  if (connect_as_printer)
    strcpy(dev,"LPT1:");

  /* send a session request (RFC 8002) */
  CVAL(outbuf,0) = 0x81;

  /* put in the destination name */
  p = outbuf+len;
  name_mangle(desthost,p);
  len += name_len(p);

  /* and my name */
  p = outbuf+len;
  name_mangle(myname,p);
  len += name_len(p);

  /* setup the packet length */
  /* We can't use smb_setlen here as it assumes a data
     packet and will trample over the name data we have copied
     in (by adding 0xFF 'S' 'M' 'B' at offsets 4 - 7 */
  CVAL(outbuf,3) = len & 0xFF;
  CVAL(outbuf,2) = (len >> 8) & 0xFF;
  if (len >= (1 << 16))
    CVAL(outbuf,1) |= 1;

  send_smb(outbuf);
  receive_smb(inbuf);
 
  if (CVAL(inbuf,0) != 0x82)
    {
      Debug(0,"Session request failed\n");
      return(False);
    }      

  memset(outbuf,0,smb_size);

  /* setup the protocol strings */
  {
    int plength;
    char *p;

    for (numprots=0,plength=0;prots[numprots].name;numprots++)
      plength += strlen(prots[numprots].name)+2;
    
    set_message(outbuf,0,plength);

    p = smb_buf(outbuf);
    for (numprots=0;prots[numprots].name;numprots++)
      {
	*p++ = 2;
	strcpy(p,prots[numprots].name);
	p += strlen(p) + 1;
      }
  }

  CVAL(outbuf,smb_com) = SMBnegprot;
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  CVAL(smb_buf(outbuf),0) = 2;

  send_smb(outbuf);
  receive_smb(inbuf);

  if (CVAL(inbuf,smb_rcls) != 0 || (SVAL(inbuf,smb_vwv0) >= numprots))
    {
      Debug(0,"SMBnegprot failed error class 0x%x code 0x%x\n",
	    CVAL(inbuf,smb_rcls),
	    SVAL(inbuf,smb_err));
      return(False);
    }

  Debug(3,"Chose protocol [%s]\n",prots[SVAL(inbuf,smb_vwv0)].name);
  Protocol = prots[SVAL(inbuf,smb_vwv0)].prot;
  if (Protocol >= PROT_COREPLUS)
    {
      readbraw_supported = ((SVAL(inbuf,smb_vwv5) & 0x1) != 0);
      writebraw_supported = ((SVAL(inbuf,smb_vwv5) & 0x2) != 0);
    }

  if (got_pass)
    pass = password;
  else
    pass = getpass("Password: ");

  /* now we've got a connection - send a tcon message */
  memset(outbuf,0,smb_size);
  set_message(outbuf,0,6 + strlen(service) + strlen(pass) + strlen(dev));
  CVAL(outbuf,smb_com) = SMBtcon;
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  p = smb_buf(outbuf);
  *p++ = 4;
  strcpy(p,service);
  p += strlen(p) + 1;
  *p++ = 4;
  strcpy(p,pass);
  p += strlen(p) + 1;
  *p++ = 4;
  strcpy(p,dev);

  send_smb(outbuf);
  receive_smb(inbuf);

  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"SMBtcon failed error class 0x%x code 0x%x\n",
	    CVAL(inbuf,smb_rcls),
	    SVAL(inbuf,smb_err));
      return(False);
    }
  

  max_xmit = SVAL(inbuf,smb_vwv0);
  max_xmit = MIN(max_xmit,BUFFER_SIZE-4);
  if (max_xmit <= 0)
    max_xmit = BUFFER_SIZE - 4;

  cnum = SVAL(inbuf,smb_vwv1);

  Debug(3,"Connected with cnum=%d max_xmit=%d\n",cnum,max_xmit);

  /* wipe out the password from memory */
  if (got_pass)
    memset(password,0,strlen(password));

  return True;

}


/****************************************************************************
send a logout command
****************************************************************************/
void send_logout(char *inbuf,char *outbuf )
{
  set_message(outbuf,0,0);

  CVAL(outbuf,smb_com) = SMBtdis;
  SSVAL(outbuf,smb_tid,cnum);
  SSVAL(outbuf,smb_pid,pid);
  SSVAL(outbuf,smb_uid,uid);
  SSVAL(outbuf,smb_mid,mid);
  send_smb(outbuf);
  receive_smb(inbuf);

  if (CVAL(inbuf,smb_rcls) != 0)
    {
      Debug(0,"SMBtdis failed error class 0x%x code 0x%x\n",
	    CVAL(inbuf,smb_rcls),
	    SVAL(inbuf,smb_err));
    }
  if (tarfile)
    cmd_endtar();
  exit(0);
}



void cmd_help();

/* This defines the commands supported by this client */
struct
{
  char *name;
  void (*fn)();
  char *description;
} commands[] = 
{
  {"ls",cmd_dir,"<mask> list the contents of the current directory"},
  {"dir",cmd_dir,"<mask> list the contents of the current directory"},
  {"lcd",cmd_lcd,"[directory] change/report the local current working directory"},
  {"cd",cmd_cd,"[directory] change/report the remote directory"},
  {"get",cmd_get,"<remote name> [local name] get a file"},
  {"mget",cmd_mget,"<mask> get all the matching files"},
  {"put",cmd_put,"<local name> [remote name] put a file"},
  {"mput",cmd_mput,"<mask> put all matching files"},
  {"del",cmd_del,"<mask> delete all matching files"},
  {"rm",cmd_del,"<mask> delete all matching files"},
  {"mkdir",cmd_mkdir,"<directory> make a directory"},
  {"md",cmd_mkdir,"<directory> make a directory"},
  {"rmdir",cmd_rmdir,"<directory> remove a directory"},
  {"rd",cmd_rmdir,"<directory> remove a directory"},
  {"prompt",cmd_prompt,"toggle prompting for filenames for mget and mput"},  
  {"recurse",cmd_recurse,"toggle directory recursion for mget and mput"},  
  {"lowercase",cmd_lowercase,"toggle lowercasing of filenames for get"},  
  {"print",cmd_print,"<file name> print a file"},
  {"printmode",cmd_printmode,"<graphics or text> set the print mode"},
  {"tar",cmd_tar,"<tarfile> Put the results of all get commands into this file"},
  {"endtar",cmd_endtar,"close the tar file, and reset the client back to normal operation"},
  {"quit",send_logout,"logoff the server"},
  {"exit",send_logout,"logoff the server"},
  {"help",cmd_help,"[command] give help on a command"},
  {"?",cmd_help,"[command] give help on a command"},
  {"",NULL}
};

/****************************************************************************
help
****************************************************************************/
void cmd_help(void)
{
  int i=0;
  char *p;

  p = strtok(NULL,SEPARATORS);
  if (p)
    {
      while (commands[i].fn)
	{
	  if (strequal(commands[i].name,p))	  
	    Debug(0,"HELP %s:\n\t%s\n\n",commands[i].name,commands[i].description);
	  i++;
	}
    }
  else
    while (commands[i].fn)
      {
	Debug(0,"%s\n",commands[i].name);
	i++;
      }
}

/****************************************************************************
open the client sockets
****************************************************************************/
BOOL open_sockets(int port )
{
  struct sockaddr_in sock_out;
  char *host;
  pstring service2;
  extern int Client;

  strcpy(service2,service);
  host = strtok(service2,"\\/");
  strcpy(desthost,host);
  if (*myname == 0)
    gethostname(myname,sizeof(myname));

  if (!have_ip)
    {
      struct hostent *hp;

      if ((hp = gethostbyname(host)) == 0) 
	{
	  Debug(0,"Gethostbyname: Unknown host %s.\n",host);
	  return False;
	}

      memcpy(&dest_ip,hp->h_addr,4);
    }

  /* create a socket to write to */
  Client = socket(PF_INET, SOCK_STREAM, 0);
  if (Client == -1) 
    {
      Debug(0,"socket error\n");
      return False;
    }
  
  memset(&sock_out, 0, sizeof(sock_out));
  memcpy(&sock_out.sin_addr, &dest_ip,4);
  
  sock_out.sin_port = htons( port );
  sock_out.sin_family = PF_INET;
  
  /* and connect it to the destination */
  if (connect(Client,(struct sockaddr *)&sock_out,sizeof(sock_out))<0)
    {
      Debug(0,"connect error\n");
      close(Client);
      return False;
    }

  return True;
}


/****************************************************************************
  process commands from the client
****************************************************************************/
void process(void )
{
  pstring line;

  InBuffer = (char *)malloc(BUFFER_SIZE);
  OutBuffer = (char *)malloc(BUFFER_SIZE);
  if ((InBuffer == NULL) || (OutBuffer == NULL)) 
    return;
  
  memset(OutBuffer,0,smb_size);
  if (!send_login(InBuffer,OutBuffer))
    return;

  while (!feof(stdin))
    {
      char *tok;
      int i;
      BOOL found = False;

      memset(OutBuffer,0,smb_size);

      /* display a prompt */
      Debug(1,"smb: %s> ", cur_dir);

      /* and get a response */
      if (!fgets(line,1000,stdin))
	break;

      /* and get the first part of the command */
      tok = strtok(line,SEPARATORS);
      
      i = 0;
      while (commands[i].fn != NULL)
	{
	  if (strequal(commands[i].name,tok))
	    {
	      found = True;
	      commands[i].fn(InBuffer,OutBuffer);
	    }
	  i++;
	}
      if (!found && tok)
	Debug(0,"%s: command not found\n",tok);
    }
  
  memset(OutBuffer,0,smb_size);
  send_logout(InBuffer,OutBuffer);
}


/****************************************************************************
usage on the program
****************************************************************************/
void usage(char *pname)
{
  Debug(0,"Usage: %s service <password> [-p port] [-d debuglevel] [-l log]\n",pname);
  Debug(0,"\t-p port               listen on the specified port\n");
  Debug(0,"\t-d debuglevel         set the debuglevel\n");
  Debug(0,"\t-l log basename.      Basename for log/debug files\n");
  Debug(0,"\t-n netbios name.      Use this name as my netbios name\n");
  Debug(0,"\t-N                    don't ask for a password\n");
  Debug(0,"\t-P                    connect to service as a printer\n");
  Debug(0,"\t-I dest IP            use this IP to connect to\n");
  Debug(0,"\t-E                    write messages to stderr instead of stdout\n");
  Debug(0,"\t-B tarbuffersize      set the buffer size for tar (in kbytes)\n");
  Debug(0,"\n");
}


/****************************************************************************
  main program
****************************************************************************/
int main(int argc,char *argv[])
{
  int port = 139;
  int opt;
  extern FILE *dbf;
  extern int DEBUGLEVEL;
  extern char *optarg;

  DEBUGLEVEL = 2;
  dbf = stdout;

  pid = getpid();
  uid = getuid();
  gid = getgid();
  myumask = umask(0);
  umask(myumask);
  
  if (argc < 2 || (*argv[1] == '-'))
    strcpy(service,"");  
  else
    {
      strcpy(service,argv[1]);  
      argc--;
      argv++;
    }

  if (argc > 1 && (*argv[1] != '-'))
    {
      got_pass = True;
      strcpy(password,argv[1]);  
      memset(argv[1],'X',strlen(argv[1]));
      argc--;
      argv++;
    }

  while ((opt = getopt (argc, argv, "Nn:d:Pp:l:hI:EB:")) != EOF)
    switch (opt)
      {
      case 'B':
	tarbuffersize = atoi(optarg)*1024;
	break;
      case 'E':
	dbf = stderr;
	break;
      case 'I':
	{
	  unsigned long a = inet_addr(optarg);
	  memcpy(&dest_ip,&a,sizeof(a));
	  have_ip = True;
	}
	break;
      case 'n':
	strcpy(myname,optarg);
	break;
      case 'N':
	got_pass = True;
	break;
      case 'P':
	connect_as_printer = True;
	break;
      case 'd':
	DEBUGLEVEL = atoi(optarg);
	break;
      case 'l':
	strcpy(debugf,optarg);
	break;
      case 'p':
	port = atoi(optarg);
	break;
      case 'h':
	usage(argv[0]);
	exit(0);
	break;
      default:
	usage(argv[0]);
	exit(1);
      }

  
  NeedSwap = big_endian();
  
  Debug(3,"%s client started\n",timestring());

  if (DEBUGLEVEL > 2)
    {
      extern FILE *login,*logout;
      pstring fname;
      sprintf(fname,"%s.client.in",debugf);
      login = fopen(fname,"w"); 
      sprintf(fname,"%s.client.out",debugf);
      logout = fopen(fname,"w");
    }

#if 0
  /* Read the broadcast address from the interface */
  get_broadcast(&myip,&bcast_ip);
#endif
  
  if (open_sockets(port))
    {
      process();
      close_sockets();
    }
  return(0);
}


#ifndef _LOADPARM_H
/* This is a dummy lp_keepalive() for the client only */
int lp_keepalive()
{
return(0);
}
#endif
