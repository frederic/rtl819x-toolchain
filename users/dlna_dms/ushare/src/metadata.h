/*
 * metadata.h : GeeXboX uShare CDS Metadata DB header.
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

#ifndef _METADATA_H_
#define _METADATA_H_

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <stdbool.h>
#include <sys/types.h>

#include "ushare.h"
#include "http.h"
#include "content.h"


struct upnp_entry_t {
  int id;
  char *fullpath;
#ifdef HAVE_DLNA
  dlna_profile_t *dlna_profile;
#endif /* HAVE_DLNA */
  struct upnp_entry_t *parent;
  int child_count;
  struct upnp_entry_t **childs;
  struct mime_type_t *mime_type;
  char *title;
  char *url;
  off_t size;
  int fd;
};

typedef struct xml_convert_s {
  char charac;
  char *xml;
} xml_convert_t;

void free_metadata_list (struct ushare_t *ut);
void build_metadata_list (struct ushare_t *ut);
struct upnp_entry_t *upnp_get_entry (struct ushare_t *ut, int id);
void upnp_entry_free (struct ushare_t *ut, struct upnp_entry_t *entry);
int rb_compare (const void *pa, const void *pb, const void *config);
struct upnp_entry_t *upnp_entry_new (struct ushare_t *ut, const char *name, const char *fullpath,struct upnp_entry_t *parent, off_t size, int dir);
void
upnp_entry_add_child (struct ushare_t *ut,
                      struct upnp_entry_t *entry, struct upnp_entry_t *child);
void metadata_add_container (struct ushare_t *ut,
                        struct upnp_entry_t *entry, const char *container);
void
metadata_add_file (struct ushare_t *ut, struct upnp_entry_t *entry,
                   const char *file, const char *name, struct stat *st_ptr);
struct upnp_entry_t *upnp_dir_new (struct ushare_t *ut, const char *name, const char *fullpath,struct upnp_entry_t *parent);
struct upnp_entry_t *upnp_file_new (struct ushare_t *ut, const char *name, const char *fullpath,struct upnp_entry_t *parent);
void
add_file (struct ushare_t *ut, struct upnp_entry_t *entry, const char *file, const char *name);

#endif /* _METADATA_H_ */
