/*
 * presentation.c : GeeXboX uShare UPnP Presentation Page.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* 2008/09/15 修改build_presentation_page()新增上傳功能
*/
#include <stdlib.h>
#include   <stdio.h>   
  #include   <ctype.h>   
  #include   <string.h>   
  #include   <dirent.h>    
  #include   <sys/types.h> 
  #include <upnp/upnp.h>
#include <upnp/upnptools.h> 
#if HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

#include "../config.h"
#include "metadata.h"
#include "content.h"
#include "buffer.h"
#include "presentation.h"
#include "gettext.h"
#include "util_iconv.h"
#include "spca/spcav4l.h"
#include "ushare.h"
#include <signal.h>

#define CGI_ACTION "action="
#define CGI_ACTION_ADD "add"
#define CGI_ACTION_DEL "del"
#define CGI_ACTION_REFRESH "refresh"
#define CGI_PATH "path"
#define CGI_SHARE "share"
#define CGI_WebCam_Start "camstart"
#define CGI_WebCam_Stop "camstop"


extern int spawn(char *prog, char **arg_list);
extern int ffmpeg_PID;
extern char *filePath_f;

extern struct vdIn videoIn;

int capFlag = 0;
static int capID = 0;
int capOK = 0;
char capFile[50];
char ffmpeg_instr[512];

   

int
process_cgi (struct ushare_t *ut, char *cgiargs)
{
  char *action = NULL;
  int refresh = 0;

  if (!ut || !cgiargs)
    return -1;

  if (strncmp (cgiargs, CGI_ACTION, strlen (CGI_ACTION)))
    return -1;

  action = cgiargs + strlen (CGI_ACTION);

  if (!strncmp (action, CGI_ACTION_ADD, strlen (CGI_ACTION_ADD)))
  {
    char *path = NULL;
    path = action + strlen (CGI_ACTION_ADD) + 1;

    if (path && !strncmp (path, CGI_PATH"=", strlen (CGI_PATH) + 1))
    {
      ut->contentlist = content_add (ut->contentlist,
                                     path + strlen (CGI_PATH) + 1);
      refresh = 1;
    }
  }
  else if (!strncmp (action, CGI_ACTION_DEL, strlen (CGI_ACTION_DEL)))
  {
    char *shares,*share;
    char *m_buffer = NULL, *buffer;
    int num, shift=0;

    shares = strdup (action + strlen (CGI_ACTION_DEL) + 1);
    m_buffer = (char*) malloc (strlen (shares) * sizeof (char));
    if (m_buffer)
    {
      buffer = m_buffer;
      for (share = strtok_r (shares, "&", &buffer) ; share ;
           share = strtok_r (NULL, "&", &buffer))
      {
        if (sscanf (share, CGI_SHARE"[%d]=on", &num) < 0)
          continue;
        ut->contentlist = content_del (ut->contentlist, num - shift++);
      }
      free (m_buffer);
    }

    refresh = 1;
    free (shares);
  }
  else if (!strncmp (action, CGI_ACTION_REFRESH, strlen (CGI_ACTION_REFRESH)))
    refresh = 1;
  else if (!strncmp (action, CGI_WebCam_Start, strlen (CGI_WebCam_Start)))
  {
  	  extern pthread_mutex_t grab_ctrl_mutex;
  	  extern pthread_cond_t grab_ctrl_cond;
	  if( videoIn.cameraname == NULL )
	  {
	  	if( init_dev(WebCam_DEV, 0)== 0)
	  	{
			pthread_mutex_lock (&grab_ctrl_mutex);
			pthread_cond_signal (&grab_ctrl_cond);
			pthread_mutex_unlock (&grab_ctrl_mutex);
			capFlag = 1;capOK = 0;
	  	}
	  	else
	  	{
	  		close_v4l(&videoIn);
	  		memset (&videoIn, 0, sizeof (struct vdIn));
	  		capFlag = 0;capOK = 1;
	  	}
  	  }
  	  else
  	  {
  	  		pthread_mutex_lock (&grab_ctrl_mutex);
			pthread_cond_signal (&grab_ctrl_cond);
			pthread_mutex_unlock (&grab_ctrl_mutex);
			capFlag = 1;capOK = 0;
  	  }
  	
  }
  else if (!strncmp (action, CGI_WebCam_Stop, strlen (CGI_WebCam_Stop)))
  {
  	extern int webcam_exit_flag,Pic_exit_flag, Mov_exit_flag;
  	extern pthread_mutex_t Pic_mutex,Mov_mutex;
  	extern pthread_cond_t Mov_cond,Pic_cond;
  	capFlag = 0;
  	if( videoIn.cameraname != NULL )
  	{
		 webcam_exit_flag = 1;
		 if( Pic_exit_flag == 1 )
		 {
		 	pthread_mutex_lock (&Pic_mutex);
			pthread_cond_signal (&Pic_cond);
			pthread_mutex_unlock (&Pic_mutex);
		 }
		 else
		 	Pic_exit_flag = 1;
		 if( Mov_exit_flag == 1 )
		 {
		 	pthread_mutex_lock (&Mov_mutex);
			pthread_cond_signal (&Mov_cond);
			pthread_mutex_unlock (&Mov_mutex);
		 }
		 else
		 	Mov_exit_flag = 1;
	}
  	
  }
  extern int scanDir_flag;
  if (refresh && ut->contentlist && !scanDir_flag)
  {
  	scanDir_flag = 1;
    free_metadata_list (ut);
    build_metadata_list (ut);
    scanDir_flag = 0;
  }

  if (ut->presentation)
    buffer_free (ut->presentation);
  ut->presentation = buffer_new ();

  buffer_append (ut->presentation, "<html>");
  buffer_append (ut->presentation, "<head>");
  buffer_appendf (ut->presentation, "<title>%s</title>",
                  _("uShare Information Page"));
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"pragma\" content=\"no-cache\"/>");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"expires\" content=\"1970-01-01\"/>");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"refresh\" content=\"0; URL=/web/ushare.html\"/>");
  buffer_append (ut->presentation, "</head>");
  buffer_append (ut->presentation, "</html>");

  return 0;
}
// ID, menuID, Title
#define MENU_DIR "<span id=\"%s\" onClick=\"showhide(%s)\" style=\"cursor:hand; font-Family:Verdana; text-decoration:underline; font-weight:bold\">%s</span><br> "
// link, Title
#define MENU_ITEM "<a href=\"%s\" target=\"_self\">%s</a><br>"
void build_item_tree(struct ushare_t *ut, struct upnp_entry_t *entry, int level)
{
	struct upnp_entry_t **childs;
	char ID[64],mID[64];
	char itemUrl[256];
	int i =0;
	if( entry )
	{
		if( entry->size <=0 )//dir
		{
			sprintf(ID, "menu%d", entry->id );
			sprintf(mID, "menu%doutline", entry->id );
			for( i =0; i < level; i++ )
			{
				buffer_append(ut->presentation,"&nbsp;&nbsp;");
			}
			if( entry == ut->root_entry )
				buffer_appendf (ut->presentation, MENU_DIR, ID, mID, "DMS");
			else
				buffer_appendf (ut->presentation, MENU_DIR, ID, mID, entry->title);
			buffer_appendf (ut->presentation, "<span id=\"%s\" style=\"display:'none'\">", mID );
			for (childs = entry->childs; *childs; childs++)
			{
				build_item_tree(ut, *childs, level+1);
			}
			buffer_append (ut->presentation, "</span>" );
		}
		else//item
		{
			for( i =0; i < level; i++ )
			{
				buffer_append(ut->presentation,"&nbsp;&nbsp;");
			}
			sprintf(itemUrl, "http://%s:%d%s/%s",UpnpGetServerIpAddress (), ut->port, VIRTUAL_DIR, entry->url);
			buffer_appendf (ut->presentation, MENU_ITEM,  itemUrl,entry->title);
		}
	}
}
int
build_presentation_page (struct ushare_t *ut)
{
  int i;
  char *mycodeset = NULL;
  char infoBuf[256];
  FILE *disk = NULL;
  char *ptr;
  long diskSize = 0;
  long freeSpace = 0;
  long useSpace = 0;
  extern int scanDir_flag;

  if (!ut)
    return -1;

  if (ut->presentation)
    buffer_free (ut->presentation);
  ut->presentation = buffer_new ();

#if HAVE_LANGINFO_CODESET
  mycodeset = nl_langinfo (CODESET);
#endif
  if (!mycodeset)
    mycodeset = UTF8;

  buffer_append (ut->presentation, "<html>");
  
  buffer_append (ut->presentation, "<head>");
  buffer_appendf (ut->presentation, "<title>%s</title>",
                 _("uShare Information Page"));
  buffer_appendf (ut->presentation,
                  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>",
                  mycodeset);
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"pragma\" content=\"no-cache\"/>");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"expires\" content=\"1970-01-01\"/>");
  buffer_append (ut->presentation, "</head>");
  buffer_append (ut->presentation, "<body onLoad=\"preload()\">");
  buffer_append (ut->presentation,"<script language=\"javascript\">\nfunction preload(){}\n");
  buffer_append (ut->presentation,"function showhide(what){\nif (what.style.display=='none'){what.style.display='';}\nelse{what.style.display='none'}}\n</script>");
  
  
  
  buffer_append (ut->presentation, "<h1 align=\"center\">");
  buffer_appendf (ut->presentation, "<tt>%s</tt><br/>",
                  _("DLNA/UPnP A/V Media Server"));
  buffer_append (ut->presentation, _("Information Page"));
  buffer_append (ut->presentation, "</h1>");
  buffer_append (ut->presentation, "<br/>");

	buffer_append (ut->presentation, "<center>");
  buffer_append (ut->presentation, "<tr width=\"500\">");
  buffer_appendf (ut->presentation, "<b>%s :</b> %s<br/>",
                  _("Version"), VERSION);
  buffer_append (ut->presentation, "</tr>");
  buffer_appendf (ut->presentation, "<b>%s :</b> %s<br/>",
                  _("Device UDN"), ut->udn);
  buffer_appendf (ut->presentation, "<b>%s :</b> %d<br/>",
                  _("Number of shared files and directories"), ut->nr_entries);
  buffer_append (ut->presentation, "</center><br/>");
///
	buffer_append (ut->presentation, "<center>");
	buffer_append (ut->presentation, "<table border=2 cellspacing=5 cellpadding=5 align=\"center\">");
	buffer_append (ut->presentation, "<tr><td>Disk Information</td>");
	if( (videoIn.cameraname == NULL) )
	{
		buffer_append (ut->presentation, "<td>Webcam does not initialize!</td></tr>");
	}
	else
	{
		buffer_appendf (ut->presentation, "<td>Found a webcam: <b>%s</b></td></tr>",videoIn.cameraname);
	}
	buffer_append (ut->presentation, "<tr><td>");
	//disk info
	system( "df /dev/sda1 > /tmp/diskinfo" );
	bzero( infoBuf,sizeof(infoBuf) );
	disk = fopen( "/tmp/diskinfo", "r" );
	if( disk )
	{
		fgets( infoBuf, sizeof( infoBuf), disk );bzero( infoBuf,sizeof(infoBuf) );
		if(!feof(disk))
			fgets( infoBuf, sizeof( infoBuf), disk );
		if( *infoBuf != '\0' )
		{
			ptr = strtok( infoBuf, " \n" );
			if( ptr != NULL ) ptr = strtok( NULL, " \n" );
			if( ptr != NULL ) { diskSize = atoi( ptr ); ptr = strtok( NULL, " \n" ); }
			if( ptr != NULL ) { useSpace = atoi( ptr ); ptr = strtok( NULL, " \n" ); }
			if( ptr != NULL ) { freeSpace = atoi( ptr ); ptr = strtok( NULL, " \n" ); }
			while( ptr )
				ptr = strtok( NULL, " \n" ); 
		}
		buffer_appendf (ut->presentation, "<b>Disk space size:</b> %dKB<br>",diskSize);
		buffer_appendf (ut->presentation, "<b>Used space size:</b> %dKB<br>",useSpace);
		buffer_appendf (ut->presentation, "<b>Free space size:</b> %dKB<br>",freeSpace);
		int picT = freeSpace/8;
		int movT = freeSpace/80;
		buffer_appendf (ut->presentation, "May store up <b>%d</b> pictures or <b>%d</b> second movies<br>",picT, movT);
		fclose( disk );
		disk = NULL;
		diskSize = 0;
		useSpace = 0;
		freeSpace = 0;
	}
	if( scanDir_flag )
	{
		buffer_append (ut->presentation, "Building metadata list, please wait few minute.<br>");
	}
	else
	{
		buffer_append (ut->presentation, "<p align=\"left\">");
		build_item_tree(ut, ut->root_entry, 0);
		buffer_append (ut->presentation, "</p>");
	}
	//
  buffer_append (ut->presentation, "</td>");
  
	
	if( (videoIn.cameraname != NULL) && (capFlag ==1) )
	{
		buffer_append (ut->presentation, "<td>");
		buffer_appendf (ut->presentation, "<applet codebase=\"http://%s/webcam/\" ",ut->ip);
		buffer_append (ut->presentation, "archive=\"webcam.jar\" code=\"webcamplayer/Main.class\" name=\"WebcamPlayer\"  align=\"center\" width=\"352\" height=\"570\">");

		buffer_appendf (ut->presentation, "<param name=\"Server\" value=\"%s\">",ut->ip);
		buffer_append (ut->presentation, "<param name=\"Port\" value=\"7070\">");

		buffer_append (ut->presentation, "<strong>You need to download Java.<br>");
		buffer_append (ut->presentation, "Click <a href=\"http://www.java.com/en/download/manual.jsp\">here:");
		buffer_append (ut->presentation, "http://www.java.com/en/download/manual.jsp</a></strong><br>");
		buffer_append (ut->presentation, "</applet>");
		if( (videoIn.cameraname != NULL) && (capFlag ==1) )
		{
			
			buffer_appendf (ut->presentation,
		              "<form method=\"get\" action=\"%s\">", USHARE_CGI);
			buffer_appendf (ut->presentation,
						      "<input type=\"hidden\" name=\"action\" value=\"%s\"/>",
						      CGI_WebCam_Stop);
			buffer_appendf (ut->presentation, "<input type=\"submit\" value=\"%s\"/>",
						      _("Stop the webcam"));
			buffer_append (ut->presentation, "</form>");
		
		}
		buffer_append (ut->presentation, "</td></tr>");
	}
	else
	{
		buffer_append (ut->presentation, "<td>");
		buffer_appendf (ut->presentation,
                  "<form method=\"get\" action=\"%s\">", USHARE_CGI);
		buffer_appendf (ut->presentation,
				          "<input type=\"hidden\" name=\"action\" value=\"%s\"/>",
				          CGI_WebCam_Start);
		if(videoIn.cameraname == NULL)
		buffer_appendf (ut->presentation, "<input type=\"submit\" value=\"%s\"/>",
				          _("Detect Webcam and start it"));
	    else
	    	buffer_appendf (ut->presentation, "<input type=\"submit\" value=\"%s\"/>",
				          _("Start webcam stream server"));
		buffer_append (ut->presentation, "</form>");
		if( capOK == 1 )
		{
			buffer_append (ut->presentation, "Can't found any webcam or not support");
			capOK = 0;
		}
		
		
		buffer_append (ut->presentation, "</td></tr>");
	}
	//upload file
	/*buffer_append (ut->presentation, "<td>");
	buffer_append (ut->presentation,"<form enctype=\"multipart/form-data\"  method=\"post\" >");
	buffer_append (ut->presentation,"<input type=\"file\" name=\"file\" size=\"20\" >");
	buffer_append (ut->presentation,_("<input type=\"Submit\" name=\"Submit\" value=\"Upload\">") );
	buffer_append (ut->presentation, "</form>");
	buffer_append (ut->presentation, "</td>");*/
	//
	buffer_append (ut->presentation, "</table>");
	//ushare
	/*buffer_appendf (ut->presentation,
                  "<form method=\"get\" action=\"%s\">", USHARE_CGI);
	buffer_appendf (ut->presentation,
	                "<input type=\"hidden\" name=\"action\" value=\"%s\"/>",
	                CGI_ACTION_DEL);
	for (i = 0 ; i < ut->contentlist->count ; i++)
	{
	buffer_appendf (ut->presentation, "<b>%s #%d :</b>", _("Share"), i + 1);
	buffer_appendf (ut->presentation,
	                "<input type=\"checkbox\" name=\""CGI_SHARE"[%d]\"/>", i);
	buffer_appendf (ut->presentation, "%s<br/>", ut->contentlist->content[i]);
	}
	buffer_appendf (ut->presentation,
	                "<input type=\"submit\" value=\"%s\"/>", _("unShare!"));
	buffer_append (ut->presentation, "</form>");
	buffer_append (ut->presentation, "<br/>");*/

	buffer_appendf (ut->presentation,
	                "<form method=\"get\" action=\"%s\">", USHARE_CGI);
	buffer_append (ut->presentation, _("Add a new share :  "));
	buffer_appendf (ut->presentation,
	                "<input type=\"hidden\" name=\"action\" value=\"%s\"/>",
	                CGI_ACTION_ADD);
	buffer_append (ut->presentation, "<input type=\"text\" name=\""CGI_PATH"\"/>");
	buffer_appendf (ut->presentation,
	                "<input type=\"submit\" value=\"%s\"/>", _("Share!"));
	buffer_append (ut->presentation, "</form>");

	buffer_append (ut->presentation, "<br/>");

	buffer_appendf (ut->presentation,
	                "<form method=\"get\" action=\"%s\">", USHARE_CGI);
	buffer_appendf (ut->presentation,
	                "<input type=\"hidden\" name=\"action\" value=\"%s\"/>",
	                CGI_ACTION_REFRESH);
	buffer_appendf (ut->presentation, "<input type=\"submit\" value=\"%s\"/>",
	                _("Refresh Shares ..."));
	buffer_append (ut->presentation, "</form>");
	///
	buffer_append (ut->presentation, "</center>");
///
  

  
  buffer_append (ut->presentation, "</center>");

  buffer_append (ut->presentation, "</body>");
  buffer_append (ut->presentation, "</html>");

  return 0;
}
