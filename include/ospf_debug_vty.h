/* OSPFd debug CLI header file
 *
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: ospf_debug_vty.h
 *
 * Purpose: header file for ospf_debug_vty.c
 */

#ifndef __ZLOG_LIST_VTY_H
#define __ZLOG_LIST_VTY_H

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vtysh/memory.h"
#include "dirs.h"
#include "util.h"
#include "daemon.h"
#include "unixctl.h"
#include "dynamic-string.h"

#define DBG_CMD_LEN_MAX    50

#define  ERR_STR\
    "Error in retrieving the mapping of feature names to daemon names"
#define DBG_STR            "Debug Configuration\n"
#define SHOW_DBG_STR       "Debugging Configuration\n"
#define OSPF_STR           "OSPF information\n"


#endif /*__ZLOG_LIST_VTY_H*/
