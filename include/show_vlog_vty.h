/* Show Vlog list CLI command file
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
 * File: vlog_list_vty.h
 *
 * Purpose: header file for vlog_list_vty.c
 */

#ifndef __VLOG_LIST_VTY_H
#define __VLOG_LIST_VTY_H

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vtysh/memory.h"
#include "dirs.h"
#include "util.h"
#include "daemon.h"
#include "unixctl.h"
#include "dynamic-string.h"
#include "showvlog.h"


#define VLOG_LIST_STR   "Show vlog supported features with log levels\n"
#define VLOG_SET_STR    "Sets the logging level of feature\n"

#define VLOG_LIST_FEATURE  "Feature name\n"
#define VLOG_CONFIG_FEATURE "configure the feature log severity\n"


#endif /*__VLOG_LIST_VTY_H*/
