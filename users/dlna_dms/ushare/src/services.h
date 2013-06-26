/*
 * services.h : GeeXboX uShare UPnP services handler header.
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
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

#ifndef _SERVICES_H_
#define _SERVICES_H_

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include "ushare.h"

struct service_action_t {
  char *name;
  bool (*function) (struct action_event_t *);
};

struct service_t {
  char *id;
  char *type;
  struct service_action_t *actions;
};

#define SERVICE_CONTENT_TYPE "text/xml"

bool find_service_action (struct Upnp_Action_Request *request,
                          struct service_t **service,
                          struct service_action_t **action);

bool upnp_add_response (struct action_event_t *event,
                        char *key, const char *value);

char * upnp_get_string (struct Upnp_Action_Request *request, const char *key);

int upnp_get_ui4 (struct Upnp_Action_Request *request, const char *key);

#endif /* _SERVICES_H_ */
