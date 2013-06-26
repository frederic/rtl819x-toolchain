/* ctrltelnet.h - Header for the Telnet controler
 * Copyright (C) 2005-2007 Sven Almgren <sven@tras.se>
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
 *
 */

#ifndef _CTRL_TELNET_H_
#define _CTRL_TELNET_H_

#define CTRL_TELNET_PORT 1337
#define CTRL_TELNET_BACKLOG 10
#define CTRL_TELNET_SHARED_BUFFER_SIZE 256
#define CTRL_CLIENT_RECV_BUFFER_SIZE 256

#include <netinet/in.h>

/**
 * @brief Structure doubling as both a connected client data holder
 *        and as a linked list
 */
typedef struct ctrl_telnet_client_t
{
  /* Recv buffer used to read single lines from more then one packet ...
     Not garanteed to be NULL terminated */
  char buffer_recv[CTRL_CLIENT_RECV_BUFFER_SIZE];

  int buffer_recv_current;
  int socket;
  int ready; /* True if this client has a complete line, ready to be parsed */
  int exiting;
  struct sockaddr_in remote_address;
  struct ctrl_telnet_client_t* next;
} ctrl_telnet_client;

typedef void (* ctrl_telnet_command_ptr) (ctrl_telnet_client *, int, char **);

/**
 * @brief Starts a Telnet bound control interface
 *
 * @return 0 on success, -1 on error
 */
int ctrl_telnet_start (int port);

/**
 * @brief Stops all telnet bound control interfaces
 */
void ctrl_telnet_stop (void);

/* FIXME: You can register a function name multiple times,
   but the last one added is the one getting called... not a problem a.t.m. */
void ctrl_telnet_register (const char *funcname,
                           ctrl_telnet_command_ptr funcptr,
                           const char* description);

int ctrl_telnet_client_send (const ctrl_telnet_client *, const char* string);
int ctrl_telnet_client_sendf (const ctrl_telnet_client *client,
                              const char* format, ...);
int ctrl_telnet_client_sendsf (const ctrl_telnet_client *client,
                               char* buffer, int buffersize,
                               const char* format, ...);

#endif /* _CTRL_TELNET_H_ */
