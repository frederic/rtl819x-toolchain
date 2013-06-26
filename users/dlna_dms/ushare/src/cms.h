/*
 * cms.h : GeeXboX uShare Connection Management Service header.
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

#ifndef CMS_H_
#define CMS_H_

#define CMS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"    <action>" \
"      <name>GetCurrentConnectionInfo</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>ConnectionID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>RcsID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>AVTransportID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>ProtocolInfo</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>PeerConnectionManager</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>PeerConnectionID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Direction</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ConnectionStatus</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetProtocolInfo</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Source</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SourceProtocolInfo</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Sink</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SinkProtocolInfo</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetCurrentConnectionIDs</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>ConnectionIDs</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"  </actionList>" \
"  <serviceStateTable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ProtocolInfo</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ConnectionStatus</name>" \
"     <dataType>string</dataType>" \
"      <allowedValueList>" \
"        <allowedValue>OK</allowedValue>" \
"        <allowedValue>ContentFormatMismatch</allowedValue>" \
"        <allowedValue>InsufficientBandwidth</allowedValue>" \
"        <allowedValue>UnreliableChannel</allowedValue>" \
"        <allowedValue>Unknown</allowedValue>" \
"      </allowedValueList>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_AVTransportID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_RcsID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ConnectionID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ConnectionManager</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>SourceProtocolInfo</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>SinkProtocolInfo</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Direction</name>" \
"      <dataType>string</dataType>" \
"      <allowedValueList>" \
"        <allowedValue>Input</allowedValue>" \
"        <allowedValue>Output</allowedValue>" \
"      </allowedValueList>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>CurrentConnectionIDs</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define CMS_DESCRIPTION_LEN strlen (CMS_DESCRIPTION)

#define CMS_LOCATION "/web/cms.xml"

#define CMS_SERVICE_ID "urn:upnp-org:serviceId:ConnectionManager"
#define CMS_SERVICE_TYPE "urn:schemas-upnp-org:service:ConnectionManager:1"

#endif /* CMS_H_ */
