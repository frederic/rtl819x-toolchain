/*
 * cfgparser.c : GeeXboX uShare config file parser headers.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Alexis Saettler <asbin@asbin.org>
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

#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include "ushare.h"

#define USHARE_NAME               "USHARE_NAME"
#define USHARE_IFACE              "USHARE_IFACE"
#define USHARE_PORT               "USHARE_PORT"
#define USHARE_TELNET_PORT        "USHARE_TELNET_PORT"
#define USHARE_DIR                "USHARE_DIR"
#define USHARE_OVERRIDE_ICONV_ERR "USHARE_OVERRIDE_ICONV_ERR"
#define USHARE_ENABLE_WEB         "USHARE_ENABLE_WEB"
#define USHARE_ENABLE_TELNET      "USHARE_ENABLE_TELNET"
#define USHARE_ENABLE_XBOX        "USHARE_ENABLE_XBOX"
#define USHARE_ENABLE_DLNA        "USHARE_ENABLE_DLNA"

#define USHARE_CONFIG_FILE        "ushare.conf"
#define DEFAULT_USHARE_NAME       "uShare"

#if (defined(BSD) || defined(__FreeBSD__))
#define DEFAULT_USHARE_IFACE      "lnc0"
#else /* Linux */
#define DEFAULT_USHARE_IFACE      "eth0"
#endif

int parse_config_file (struct ushare_t *ut)
    __attribute__ ((nonnull));
int parse_command_line (struct ushare_t *ut, int argc, char **argv)
    __attribute__ ((nonnull (1)));

typedef struct {
  char *name;
  void (*set_var) (struct ushare_t*, const char*);
} u_configline_t;

#endif /* _CONFIG_PARSER_H_ */
