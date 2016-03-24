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

#define SHOW_VLOG_STR            "Display all vlogs\n"
#define SHOW_VLOG_CONFIG_STR     "Display vlog configurations\n"
#define SHOW_VLOG_NAME           "Specify the feature or ops-daemon name\n"
#define SHOW_VLOG_LIST_FEATURE   "Display vlog supported features list\n"
#define SHOW_VLOG_FEATURE        "Displays feature vlog configuration\n"
#define SHOW_VLOG_DAEMON         "Displays ops-daemon vlog configurations\n"
#define SHOW_VLOG_FILTER_SEV     "Display vlogs for specified severity\n"
#define SHOW_VLOG_FILTER_DAEMON  "Display vlogs for specified ops-daemon\n"
#define SHOW_VLOG_FILTER_WORD    "Display logs for specified ops-daemon\n"
#define VLOG_CONFIG_FEATURE      "Configure the feature\n"
#define VLOG_CONFIG_DAEMON       "Configure the ops-daemon\n"
#define VLOG_CONFIG              "Configure the feature or ops-daemon\n"
#define VLOG_LOG_DEST_SYSLOG     "Configure syslog log level for the feature or ops-daemon\n"
#define VLOG_LOG_DEST_FILE       "Configure file log level for feature or ops-daemon\n"
#define VLOG_LOG_DEST_ALL        "Configure both syslog and file log level for feature or ops-daemon\n"
#define VLOG_LOG_LEVEL_EMER      "Configure to send emer logs to specified destination\n"
#define VLOG_LOG_LEVEL_ERR       "Configure to send emer and err logs to specified destination\n"
#define VLOG_LOG_LEVEL_WARN      "Configure to send emer, err and warn logs to specified destination\n"
#define VLOG_LOG_LEVEL_INFO      "Configure to send emer,err,warn and info logs to specified destination\n"
#define VLOG_LOG_LEVEL_DBG       "Configure to send emer,err,warn,info and dbg logs to specified destination\n"
#define VLOG_LOG_LEVEL_OFF       "Disable logging to specified destination\n"

#endif /*__VLOG_LIST_VTY_H*/
