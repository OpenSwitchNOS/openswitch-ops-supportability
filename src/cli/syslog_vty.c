/* System SYSLOG CLI commands
*
* Copyright (C) 1997, 98 Kunihiro Ishiguro
* Copyright (C) 2016 Hewlett Packard Enterprise Development LP
*
* GNU Zebra is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2, or (at your option) any
* later version.
*
* GNU Zebra is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with GNU Zebra; see the file COPYING.  If not, write to the Free
* Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
* 02111-1307, USA.
*
* File: syslog_vty.c
*
* Purpose: To config syslog from CLI
*/

#include "vtysh/command.h"
#include "vtysh_ovsdb_if.h"
#include "vtysh_ovsdb_config.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "syslog_vty.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "smap.h"
#include "vtysh/memory.h"
#include "openvswitch/vlog.h"
#include "dynamic-string.h"
#include "vtysh/buffer.h"
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "supportability_utils.h"

VLOG_DEFINE_THIS_MODULE (vtysh_syslog_cli);
extern struct ovsdb_idl *idl;

#define VLOG_ERR_SYSLOG_TRANSACTION_COMMIT_FAILED VLOG_ERR("syslog remote : transaction commit failed \n")
#define VLOG_ERR_SYSLOG_INSERT_FAILED VLOG_ERR("syslog remote : inserting new row failed \n")
#define VLOG_ERR_SYSLOG_OPENVSWITCH_READ_FAILED VLOG_ERR("syslog remote : DB read failed \n")
#define VLOG_ERR_SYSLOG_TRANSACTION_CREATE_FAILED  VLOG_ERR(OVSDB_TXN_CREATE_ERROR)


const struct ovsrec_syslog_remote*
syslog_remote_get_config(const char* remote_host,
              const char* transport,
              const int64_t* port_number,
              const char* severity)
{

    const struct ovsrec_syslog_remote *row= NULL;
    const struct ovsrec_syslog_remote *next= NULL;
    OVSREC_SYSLOG_REMOTE_FOR_EACH_SAFE(row,next,idl)
    {
        /* Compare Host name */
        if(strcmp_with_nullcheck(row->remote_host,remote_host))
        {
            continue;
        }
        /* Compare Transport */
        if(row->transport != NULL && transport != NULL)
        {
            if(strcmp_with_nullcheck(row->transport,transport))
            {
                continue;
            }
        }
        else if(row->transport == NULL && transport != NULL)
        {
            if(strcmp_with_nullcheck("udp",transport))
            {
                continue;
            }
        }
        else if(row->transport != NULL && transport == NULL)
        {
            if(strcmp_with_nullcheck(row->transport,"udp"))
            {
                continue;
            }
        }
        /* If both the transport are NULL then they match , hence no check*/

        /* For Future reference
           Compare Port_Number
           if(row->port_number == NULL && port_number != NULL)
           {
           continue;
           }
           else if (row->port_number != NULL && port_number == NULL)
           {
           continue;
           }
           else if (row->port_number != NULL && port_number != NULL)
           {
           if(*row->port_number != *port_number)
           {
           continue;
           }
           }
           Compare severity
           if(row->severity != NULL || severity != NULL)
           {
           if(strcmp_with_nullcheck(row->severity,severity))
           {
           continue;
           }
           }
           */
        return row;
    }
   return NULL;
}

int
cli_syslog_delete_config(const char* remote_host,
              const char* transport,
              const int64_t* port_number,
              const char* severity,
              const struct ovsrec_system *system_row
        )
{
    int i,n;
    const struct ovsrec_syslog_remote *row = NULL;
    struct ovsrec_syslog_remote **row_array = NULL;

    row = syslog_remote_get_config(remote_host,transport,port_number,severity);
    /* If the config exists, then delete the roew */
    if(row != NULL)
    {
        ovsrec_syslog_remote_delete(row);

        /* Remove its association from the parent table (system table) */
        /* Update the system table */
        row_array = (struct ovsrec_syslog_remote **)
            calloc(sizeof(struct ovsrec_syslog_remote *),
                    system_row->n_syslog_remotes-1);
        if(row_array == NULL)
        {
            VLOG_ERR_SYSLOG_INSERT_FAILED;
            return CMD_OVSDB_FAILURE;
        }

        for( i = n =0;i < system_row->n_syslog_remotes;i++ )
        {
            if(system_row->syslog_remotes[i] != row)
            {
                row_array[n++] = system_row->syslog_remotes[i];
            }
        }
        ovsrec_system_verify_syslog_remotes(system_row);
        ovsrec_system_set_syslog_remotes(system_row,
                row_array,
                n);
        free(row_array);
    }
    return CMD_SUCCESS;
}


int64_t
cli_syslog_config(const char* remote_host,
              const char* transport,
              const int64_t* port_number,
              const char* severity,
              int is_add)
{
    int i = 0;
    int returncode;
    struct ovsrec_syslog_remote *row = NULL;
    struct ovsrec_syslog_remote **row_array = NULL;
    const struct ovsrec_system *system_row = NULL;
    enum ovsdb_idl_txn_status txn_status;
    struct ovsdb_idl_txn *txn = cli_do_config_start();

    if(txn == NULL)
    {
        VLOG_ERR_SYSLOG_TRANSACTION_CREATE_FAILED;
        cli_do_config_abort(txn);
        return CMD_OVSDB_FAILURE;
    }

    if(remote_host == NULL)
    {
        vty_out(vty,"Command failed%s",VTY_NEWLINE);
        cli_do_config_abort(txn);
        return CMD_WARNING;
    }

    system_row = ovsrec_system_first(idl);
    if(system_row == NULL)
    {
        VLOG_ERR_SYSLOG_OPENVSWITCH_READ_FAILED;
        cli_do_config_abort(txn);
        return CMD_OVSDB_FAILURE;
    }

    /* Delete if similar config already exists */
    returncode = cli_syslog_delete_config(remote_host,transport,
            port_number,severity,
            system_row);
    if(returncode != CMD_SUCCESS)
    {
        VLOG_ERR("Delete syslog config failed");
        cli_do_config_abort(txn);
        return returncode;
    }

    if(is_add)
    {
        /* Check the number of syslog remotes already configured.
           We support only upto 4 configuration.
           */
        if(system_row->n_syslog_remotes >= 4)
        {
            vty_out(vty, "Limit of 4 remote syslog servers already reached%s",VTY_NEWLINE);
            cli_do_config_abort(txn);
            return CMD_WARNING;
        }

        /* Insert new row in the syslog_remote table */
        row = ovsrec_syslog_remote_insert(txn);

        if(row == NULL)
        {
            VLOG_ERR_SYSLOG_INSERT_FAILED;
            cli_do_config_abort(txn);
            return CMD_OVSDB_FAILURE;
        }

        /* Update remote host*/
        ovsrec_syslog_remote_verify_remote_host(row);
        ovsrec_syslog_remote_set_remote_host(row, remote_host);

        /* Update transport protocol if provided */
        if(transport != NULL)
        {
            ovsrec_syslog_remote_verify_transport(row);
            ovsrec_syslog_remote_set_transport(row, transport);
        }
        /* Update port number if provided */
        if(port_number != NULL && *port_number > 0 && *port_number <= 65535)
        {
            ovsrec_syslog_remote_verify_port_number(row);
            ovsrec_syslog_remote_set_port_number(row, port_number,1);
        }
        /* Update severity if provided */
        if(severity != NULL)
        {
            ovsrec_syslog_remote_verify_severity(row);
            ovsrec_syslog_remote_set_severity(row, severity);
        }
        /* Update the system table */
        row_array = (struct ovsrec_syslog_remote **)
            calloc(sizeof(struct ovsrec_syslog_remote *),
                    system_row->n_syslog_remotes+1);
        if(row_array == NULL)
        {
            VLOG_ERR_SYSLOG_INSERT_FAILED;
            cli_do_config_abort(txn);
            return CMD_OVSDB_FAILURE;
        }


        for(i = 0; i < system_row->n_syslog_remotes;i++)
        {
            row_array[i] = system_row->syslog_remotes[i];
        }
        row_array[i] = row;
        ovsrec_system_verify_syslog_remotes(system_row);
        ovsrec_system_set_syslog_remotes(system_row,
                row_array,
                system_row->n_syslog_remotes+1);
    }
    txn_status = cli_do_config_finish(txn);
    if(row_array)
    {
        free(row_array);
        row_array= NULL;
    }
    if(txn_status != TXN_SUCCESS && txn_status != TXN_UNCHANGED)
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        vty_out(vty,"Configuration failed%s",VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }
    return CMD_SUCCESS;

}

DEFUN (vtysh_config_syslog_basic,
       vtysh_config_syslog_basic_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       )
{
    return cli_syslog_config(argv[0],NULL,NULL,NULL,1);
}

DEFUN (vtysh_config_syslog_udp,
       vtysh_config_syslog_udp_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) udp [<1-65535>]",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       UDP_PORT_STR
       )
{
    const int64_t* port_number = NULL;
    int64_t port;
    if(argc == 2)
    {
        /* If optional port_number is provided */
        port =(int64_t) atoi(argv[1]);
        port_number = &port;
    }
    return cli_syslog_config(argv[0],"udp",port_number,NULL,1);
}


DEFUN (vtysh_config_syslog_tcp,
       vtysh_config_syslog_tcp_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) tcp [<1-65535>]",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       TCP_STR
       TCP_PORT_STR
       )
{
    const int64_t* port_number = NULL;
    int64_t port;
    if(argc == 2)
    {
        /* If optional port_number is provided */
        port =(int64_t) atoi(argv[1]);
        port_number = &port;
    }
    return cli_syslog_config(argv[0],"tcp",port_number,NULL,1);
}

DEFUN (vtysh_config_syslog_svrt,
       vtysh_config_syslog_svrt_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
       )
{
    return cli_syslog_config(argv[0],NULL,NULL,argv[1],1);
}

DEFUN (vtysh_config_syslog_udp_svrt,
       vtysh_config_syslog_udp_svrt_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) udp [<1-65535>] severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       UDP_PORT_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    const int64_t* port_number = NULL;
    int64_t port;
    port = (int64_t)atoi(argv[1]);
    port_number = &port;

    return cli_syslog_config(argv[0],"udp",port_number,argv[2],1);
}

DEFUN (vtysh_config_syslog_tcp_svrt,
       vtysh_config_syslog_tcp_svrt_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) tcp [<1-65535>] severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       TCP_STR
       TCP_PORT_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    const int64_t* port_number = NULL;
    int64_t port;
    port = (int64_t)atoi(argv[1]);
    port_number = &port;

    return cli_syslog_config(argv[0],"tcp",port_number,argv[2],1);
}

DEFUN (vtysh_config_syslog_prot_svrt_noport,
       vtysh_config_syslog_prot_svrt_noport_cmd,
       "logging (A.B.C.D | X:X::X:X | WORD) (udp|tcp) severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       TCP_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    return cli_syslog_config(argv[0],argv[1],NULL,argv[2],1);
}

/* No part of the commands */

DEFUN (no_vtysh_config_syslog_basic,
       no_vtysh_config_syslog_basic_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       )
{
    return cli_syslog_config(argv[0],NULL,NULL,NULL,0);
}

DEFUN (no_vtysh_config_syslog_udp,
       no_vtysh_config_syslog_udp_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) udp [<1-65535>]",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       UDP_PORT_STR
       )
{
    const int64_t* port_number = NULL;
    int64_t port;
    if(argc == 2)
    {
        /* If optional port_number is provided */
        port =(int64_t) atoi(argv[1]);
        port_number = &port;
    }
    return cli_syslog_config(argv[0],"udp",port_number,NULL,0);
}


DEFUN (no_vtysh_config_syslog_tcp,
       no_vtysh_config_syslog_tcp_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) tcp [<1-65535>]",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       TCP_STR
       TCP_PORT_STR
       )
{
    const int64_t* port_number = NULL;
    int64_t port;
    if(argc == 2)
    {
        /* If optional port_number is provided */
        port =(int64_t) atoi(argv[1]);
        port_number = &port;
    }
    return cli_syslog_config(argv[0],"tcp",port_number,NULL,0);
}

DEFUN (no_vtysh_config_syslog_svrt,
       no_vtysh_config_syslog_svrt_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
       )
{
    return cli_syslog_config(argv[0],NULL,NULL,argv[1],0);
}

DEFUN (no_vtysh_config_syslog_udp_svrt,
       no_vtysh_config_syslog_udp_svrt_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) udp [<1-65535>] severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       UDP_PORT_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    const int64_t* port_number = NULL;
    int64_t port;
    port = (int64_t)atoi(argv[1]);
    port_number = &port;

    return cli_syslog_config(argv[0],"udp",port_number,argv[2],0);
}

DEFUN (no_vtysh_config_syslog_tcp_svrt,
       no_vtysh_config_syslog_tcp_svrt_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) tcp [<1-65535>] severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       TCP_STR
       TCP_PORT_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    const int64_t* port_number = NULL;
    int64_t port;
    port = (int64_t)atoi(argv[1]);
    port_number = &port;

    return cli_syslog_config(argv[0],"tcp",port_number,argv[2],0);
}

DEFUN (no_vtysh_config_syslog_prot_svrt_noport,
       no_vtysh_config_syslog_prot_svrt_noport_cmd,
       "no logging (A.B.C.D | X:X::X:X | WORD) (udp|tcp) severity (debug|info|notice|warning|err|crit|alert|emerg)",
       LOGGING_STR
       HOST_IPv4
       HOST_IPv6
       HOSTNAME_STR
       UDP_STR
       TCP_STR
       SEVERITY_STR
       DEBUG_SVR_STR
       INFO_STR
       NOTICE_STR
       WARNING_STR
       ERR_STR
       CRIT_STR
       ALERT_STR
       EMERG_STR
     )
{
    return cli_syslog_config(argv[0],argv[1],NULL,argv[2],0);
}



void
syslog_ovsdb_init(void)
{
    ovsdb_idl_add_table (idl, &ovsrec_table_syslog_remote);
    ovsdb_idl_add_column(idl, &ovsrec_syslog_remote_col_remote_host);
    ovsdb_idl_add_column(idl, &ovsrec_syslog_remote_col_severity);
    ovsdb_idl_add_column(idl, &ovsrec_syslog_remote_col_transport);
    ovsdb_idl_add_column(idl, &ovsrec_syslog_remote_col_port_number);
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_column(idl, &ovsrec_system_col_syslog_remotes);
    return ;
}
