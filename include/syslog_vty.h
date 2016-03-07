/* SYSLOG CLI commands.
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
 * File: syslog_vty.h
 *
 * Purpose: To configure syslog from CLI.
 */

#ifndef _SYSLOG_VTY_H
#define _SYSLOG_VTY_H

#define LOGGING_STR \
"Configure remote syslog servers receiving syslog messages\n"
#define HOST_IPv4 \
"IPv4 address of the remote syslog server\n"
#define HOST_IPv6 \
"IPv6 address of the remote syslog server\n"
#define HOSTNAME_STR \
"FQDN or hostname string (Max Length 256) of the remote syslog server\n"
#define UDP_STR \
"Forward syslog messages using UDP protocol(Default)\n"
#define TCP_STR \
"Forward syslog messages using TCP protocol\n"
#define UDP_PORT_STR \
"UDP Port on which remote syslog server is listening(Default:514)\n"
#define TCP_PORT_STR \
"TCP Port on which remote syslog server is listening(Default:1470)\n"
#define SEVERITY_STR \
"Syslog messages of the specified severity or higher will"\
" be sent to the syslog server\n"
#define DEBUG_SVR_STR \
"Forward syslog message of debug or higher severity to syslog server(Default)\n"
#define INFO_STR \
"Forward syslog message of info or higher severity to syslog server\n"
#define NOTICE_STR \
"Forward syslog message of notice or higher severity to syslog server\n"
#define WARNING_STR \
"Forward syslog message of warning or higher severity to syslog server\n"
#define ERR_STR \
"Forward syslog message of err or higher severity to syslog server\n"
#define CRIT_STR \
"Forward syslog message of crit or higher severity to syslog server\n"
#define ALERT_STR \
"Forward syslog message of alert or higher severity to syslog server\n"
#define EMERG_STR \
"Forward syslog message of emerg or higher severity to syslog server\n"


#endif //_SYSLOG_VTY_H
