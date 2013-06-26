/*
 * check.c - validation of DNS packets
 *
 * Copyright (C) 2004 Natanael Copa <n@tanael.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
#include "query.h"
#include "relay.h"
#include "cache.h"
#include "common.h"
#include "tcp.h"
#include "udp.h"
#include "master.h"
*/
#include "check.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


/*
 * valid_dnspacket()
 *
 * This functions does some test to verify that the message is sane
 * returns < 0 if insane message
 *
 */

static int valid_dnspacket(int len, void *msg) {
  
}
