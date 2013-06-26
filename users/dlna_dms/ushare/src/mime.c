/*
 * mime.c : GeeXboX uShare media file MIME-type association.
 * Originally developped for the GeeXboX project.
 * Ref : http://freedesktop.org/wiki/Standards_2fshared_2dmime_2dinfo_2dspec
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

#include <stdlib.h>
#include <string.h>

#include "mime.h"
#include "ushare.h"

#define UPNP_VIDEO "object.item.videoItem"
#define UPNP_AUDIO "object.item.audioItem.musicTrack"
#define UPNP_PHOTO "object.item.imageItem.photo"
#define UPNP_PLAYLIST "object.item.playlistItem"
#define UPNP_TEXT "object.item.textItem"

const struct mime_type_t MIME_Type_List[] = {
  /* Video files */
  { "asf",   UPNP_VIDEO, "http-get:*:video/x-ms-asf:"},
  { "avc",   UPNP_VIDEO, "http-get:*:video/avi:"},
  { "avi",   UPNP_VIDEO, "http-get:*:video/avi:"},
  { "dv",    UPNP_VIDEO, "http-get:*:video/x-dv:"},
  { "divx",  UPNP_VIDEO, "http-get:*:video/avi:"},
  { "wmv",   UPNP_VIDEO, "http-get:*:video/x-ms-wmv:"},
  { "mjpg",  UPNP_VIDEO, "http-get:*:video/x-motion-jpeg:"},
  { "mjpeg", UPNP_VIDEO, "http-get:*:video/x-motion-jpeg:"},
  { "mpeg",  UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "mpg",   UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "mpe",   UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "mp2p",  UPNP_VIDEO, "http-get:*:video/mp2p:"},
  { "vob",   UPNP_VIDEO, "http-get:*:video/mp2p:"},
  { "mp2t",  UPNP_VIDEO, "http-get:*:video/mp2t:"},
  { "m1v",   UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "m2v",   UPNP_VIDEO, "http-get:*:video/mpeg2:"},
  { "mpg2",  UPNP_VIDEO, "http-get:*:video/mpeg2:"},
  { "mpeg2", UPNP_VIDEO, "http-get:*:video/mpeg2:"},
  { "m4v",   UPNP_VIDEO, "http-get:*:video/mp4:"},
  { "m4p",   UPNP_VIDEO, "http-get:*:video/mp4:"},
  { "mp4ps", UPNP_VIDEO, "http-get:*:video/x-nerodigital-ps:"},
  { "ts",    UPNP_VIDEO, "http-get:*:video/mpeg2:"},
  { "ogm",   UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "mkv",   UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "rmvb",  UPNP_VIDEO, "http-get:*:video/mpeg:"},
  { "mov",   UPNP_VIDEO, "http-get:*:video/quicktime:"},
  { "hdmov", UPNP_VIDEO, "http-get:*:video/quicktime:"},
  { "qt",    UPNP_VIDEO, "http-get:*:video/quicktime:"},
  { "bin",   UPNP_VIDEO, "http-get:*:video/mpeg2:"},
  { "iso",   UPNP_VIDEO, "http-get:*:video/mpeg2:"},

  /* Audio files */
  { "3gp",  UPNP_AUDIO, "http-get:*:audio/3gpp:"},
  { "aac",  UPNP_AUDIO, "http-get:*:audio/x-aac:"},
  { "ac3",  UPNP_AUDIO, "http-get:*:audio/x-ac3:"},
  { "aif",  UPNP_AUDIO, "http-get:*:audio/aiff:"},
  { "aiff", UPNP_AUDIO, "http-get:*:audio/aiff:"},
  { "at3p", UPNP_AUDIO, "http-get:*:audio/x-atrac3:"},
  { "au",   UPNP_AUDIO, "http-get:*:audio/basic:"},
  { "snd",  UPNP_AUDIO, "http-get:*:audio/basic:"},
  { "dts",  UPNP_AUDIO, "http-get:*:audio/x-dts:"},
  { "rmi",  UPNP_AUDIO, "http-get:*:audio/midi:"},
  { "mid",  UPNP_AUDIO, "http-get:*:audio/midi:"},
  { "mp1",  UPNP_AUDIO, "http-get:*:audio/mp1:"},
  { "mp2",  UPNP_AUDIO, "http-get:*:audio/mp2:"},
  { "mp3",  UPNP_AUDIO, "http-get:*:audio/mpeg:"},
  { "mp4",  UPNP_AUDIO, "http-get:*:audio/mp4:"},
  { "m4a",  UPNP_AUDIO, "http-get:*:audio/mp4:"},
  { "ogg",  UPNP_AUDIO, "http-get:*:audio/x-ogg:"},
  { "wav",  UPNP_AUDIO, "http-get:*:audio/wav:"},
  { "pcm",  UPNP_AUDIO, "http-get:*:audio/l16:"},
  { "lpcm", UPNP_AUDIO, "http-get:*:audio/l16:"},
  { "l16",  UPNP_AUDIO, "http-get:*:audio/l16:"},
  { "wma",  UPNP_AUDIO, "http-get:*:audio/x-ms-wma:"},
  { "mka",  UPNP_AUDIO, "http-get:*:audio/mpeg:"},
  { "ra",   UPNP_AUDIO, "http-get:*:audio/x-pn-realaudio:"},
  { "rm",   UPNP_AUDIO, "http-get:*:audio/x-pn-realaudio:"},
  { "ram",  UPNP_AUDIO, "http-get:*:audio/x-pn-realaudio:"},
  { "flac", UPNP_AUDIO, "http-get:*:audio/x-flac:"},

  /* Images files */
  { "bmp",  UPNP_PHOTO, "http-get:*:image/bmp:"},
  { "ico",  UPNP_PHOTO, "http-get:*:image/x-icon:"},
  { "gif",  UPNP_PHOTO, "http-get:*:image/gif:"},
  { "jpeg", UPNP_PHOTO, "http-get:*:image/jpeg:"},
  { "jpg",  UPNP_PHOTO, "http-get:*:image/jpeg:"},
  { "jpe",  UPNP_PHOTO, "http-get:*:image/jpeg:"},
  { "pcd",  UPNP_PHOTO, "http-get:*:image/x-ms-bmp:"},
  { "png",  UPNP_PHOTO, "http-get:*:image/png:"},
  { "pnm",  UPNP_PHOTO, "http-get:*:image/x-portable-anymap:"},
  { "ppm",  UPNP_PHOTO, "http-get:*:image/x-portable-pixmap:"},
  { "qti",  UPNP_PHOTO, "http-get:*:image/x-quicktime:"},
  { "qtf",  UPNP_PHOTO, "http-get:*:image/x-quicktime:"},
  { "qtif", UPNP_PHOTO, "http-get:*:image/x-quicktime:"},
  { "tif",  UPNP_PHOTO, "http-get:*:image/tiff:"},
  { "tiff", UPNP_PHOTO, "http-get:*:image/tiff:"},

  /* Playlist files */
  { "pls", UPNP_PLAYLIST, "http-get:*:audio/x-scpls:"},
  { "m3u", UPNP_PLAYLIST, "http-get:*:audio/mpegurl:"},
  { "asx", UPNP_PLAYLIST, "http-get:*:video/x-ms-asf:"},

  /* Subtitle Text files */
  { "srt", UPNP_TEXT, "http-get:*:text/srt:"}, /* SubRip */
  { "ssa", UPNP_TEXT, "http-get:*:text/ssa:"}, /* SubStation Alpha */
  { "stl", UPNP_TEXT, "http-get:*:text/srt:"}, /* Spruce */
  { "psb", UPNP_TEXT, "http-get:*:text/psb:"}, /* PowerDivX */
  { "pjs", UPNP_TEXT, "http-get:*:text/pjs:"}, /* Phoenix Japanim */
  { "sub", UPNP_TEXT, "http-get:*:text/sub:"}, /* MicroDVD */
  { "idx", UPNP_TEXT, "http-get:*:text/idx:"}, /* VOBsub */
  { "dks", UPNP_TEXT, "http-get:*:text/dks:"}, /* DKS */
  { "scr", UPNP_TEXT, "http-get:*:text/scr:"}, /* MACsub */
  { "tts", UPNP_TEXT, "http-get:*:text/tts:"}, /* TurboTitler */
  { "vsf", UPNP_TEXT, "http-get:*:text/vsf:"}, /* ViPlay */
  { "zeg", UPNP_TEXT, "http-get:*:text/zeg:"}, /* ZeroG */
  { "mpl", UPNP_TEXT, "http-get:*:text/mpl:"}, /* MPL */

  /* Miscellaneous text files */
  { "bup", UPNP_TEXT, "http-get:*:text/bup:"}, /* DVD backup */
  { "ifo", UPNP_TEXT, "http-get:*:text/ifo:"}, /* DVD information */

  { NULL, NULL, NULL}
};

char *mime_get_protocol (struct mime_type_t *mime)
{
  char protocol[512];
  
  if (!mime)
    return NULL;

  sprintf (protocol, mime->mime_protocol);
  strcat (protocol, "*");
  return strdup (protocol);
}
