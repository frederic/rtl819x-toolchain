/*
 * services.c : GeeXboX uShare UPnP services handler.
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

#include <stdlib.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "ushare.h"
#include "services.h"
#include "cms.h"
#include "cds.h"
#include "msr.h"
#include "trace.h"

/* Represent the ObjectID argument. */
#define ARG_OBJECT_ID "ObjectID"

/* Represent the ContainerID argument. */
#define ARG_CONTAINER_ID "ContainerID"

extern struct service_action_t cds_service_actions[];
extern struct service_action_t cms_service_actions[];
extern struct service_action_t msr_service_actions[];

static struct service_t services[] = {
  {
    CDS_SERVICE_ID,
    CDS_SERVICE_TYPE,
    cds_service_actions
  },
  {
    CMS_SERVICE_ID,
    CMS_SERVICE_TYPE,
    cms_service_actions
  },
  {
    MSR_SERVICE_ID,
    MSR_SERVICE_TYPE,
    msr_service_actions
  },
  { NULL, NULL, NULL }
};

bool
find_service_action (struct Upnp_Action_Request *request,
                     struct service_t **service,
                     struct service_action_t **action)
{
  int c, d;

  *service = NULL;
  *action = NULL;

  if (!request || !request->ActionName)
    return false;

  for (c = 0; services[c].id != NULL; c++)
    if (!strcmp (services[c].id, request->ServiceID))
    {
      *service = &services[c];
      for (d = 0; services[c].actions[d].name; d++)
      {
        if (!strcmp (services[c].actions[d].name, request->ActionName))
        {
          *action = &services[c].actions[d];
          return true;
        }
      }
      return false;
    }

  return false;
}

bool
upnp_add_response (struct action_event_t *event, char *key, const char *value)
{
  char *val;
  int res;

  if (!event || !event->status || !key || !value)
    return false;

  val = strdup (value);
  if (!val)
    return false;

  res = UpnpAddToActionResponse (&event->request->ActionResult,
                                 event->request->ActionName,
                                 event->service->type, key, val);

  if (res != UPNP_E_SUCCESS)
    {
      free (val);
      return false;
    }

  free (val);
  return true;
}

char *
upnp_get_string (struct Upnp_Action_Request *request, const char *key)
{
  IXML_Node *node = NULL;

  if (!request || !request->ActionRequest || !key)
    return NULL;

  node = (IXML_Node *) request->ActionRequest;
  if (!node)
  {
    log_verbose ("Invalid action request document\n");
    return NULL;
  }

  node = ixmlNode_getFirstChild (node);
  if (!node)
  {
    log_verbose ("Invalid action request document\n");
    return NULL;
  }

  node = ixmlNode_getFirstChild (node);
  for (; node; node = ixmlNode_getNextSibling (node))
    if (!strcmp (ixmlNode_getNodeName (node), key))
    {
      node = ixmlNode_getFirstChild (node);
      if (!node)
        return strdup ("");
      return strdup (ixmlNode_getNodeValue (node));
    }

  log_verbose ("Missing action request argument (%s)\n", key);

  return NULL;
}

int
upnp_get_ui4 (struct Upnp_Action_Request *request, const char *key)
{
  char *value;
  int val;

  if (!request || !key)
    return 0;

  value = upnp_get_string (request, key);
  if (!value && !strcmp (key, ARG_OBJECT_ID))
    value = upnp_get_string (request, ARG_CONTAINER_ID);

  if (!value)
    return 0;

  val = atoi (value);
  free (value);

  return val;
}
