/*
 * msr.c : GeeXboX uShare Microsoft Registrar Service.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2006 Benjamin Zores <ben@geexbox.org>
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

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "ushare.h"
#include "services.h"

/* Represent the MSR IsAuthorized action. */
#define SERVICE_MSR_ACTION_IS_AUTHORIZED "IsAuthorized"

/* Represent the MSR RegisterDevice action. */
#define SERVICE_MSR_ACTION_REGISTER_DEVICE "RegisterDevice"

/* Represent the MSR IsValidated action. */
#define SERVICE_MSR_ACTION_IS_VALIDATED "IsValidated"

/* Represent the MSR DeviceID argument. */
#define SERVICE_MSR_ARG_DEVICE_ID "DeviceID"

/* Represent the MSR Result argument. */
#define SERVICE_MSR_ARG_RESULT "Result"

/* Represent the MSR RegistrationReqMsg argument. */
#define SERVICE_MSR_ARG_REGISTRATION_REQUEST_MSG "RegistrationReqMsg"

/* Represent the MSR RegistrationRespMsg argument. */
#define SERVICE_MSR_ARG_REGISTRATION_RESPONSE_MSG "RegistrationRespMsg"

/* Represent the MSR Registered/Activated ID value. */
#define SERVICE_MSR_STATUS_OK "1"

static bool
msr_is_authorized (struct action_event_t *event)
{
  if (!event)
    return false;

  /* send a fake authorization to these stupid MS players ;-) */
  upnp_add_response (event, SERVICE_MSR_ARG_RESULT, SERVICE_MSR_STATUS_OK);

  return event->status;
}

static bool
msr_register_device (struct action_event_t *event)
{
  if (!event)
    return false;

  /* dummy action */

  return event->status;
}

static bool
msr_is_validated (struct action_event_t *event)
{
  if (!event)
    return false;

  /* send a fake validation to these stupid MS players ;-) */
  upnp_add_response (event, SERVICE_MSR_ARG_RESULT, SERVICE_MSR_STATUS_OK);

  return event->status;
}

/* List of UPnP Microsoft Registrar Service actions */
struct service_action_t msr_service_actions[] = {
  { SERVICE_MSR_ACTION_IS_AUTHORIZED, msr_is_authorized },
  { SERVICE_MSR_ACTION_REGISTER_DEVICE, msr_register_device },
  { SERVICE_MSR_ACTION_IS_VALIDATED, msr_is_validated },
  { NULL, NULL }
};
