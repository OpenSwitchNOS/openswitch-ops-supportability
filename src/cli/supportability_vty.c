/* System SUPPORTABILITY CLI commands
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
* File: supportability_vty.c
*
* Purpose: To Install all Supportability CLI Commands
*/

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vtysh/memory.h"
#include "dynamic-string.h"
#include "supportability_vty.h"
#include "vtysh_ovsdb_syslog_context.h"
#include "vtysh/vtysh_ovsdb_config.h"
/*
 * Function           : cli_pre_init
 * Responsibility     : Install the cli nodes
 */

void
cli_pre_init(void)
{
   /* Supportability Doesnt have any new node */
    syslog_ovsdb_init();
}

/*
 * Function           : cli_post_init
 * Responsibility     : Install the cli action routines
 *                      This function is common across all supportability cli
 *                      Install all supportability cli here
 */
void
cli_post_init()
{
  vtysh_ret_val retval = e_vtysh_error;
  install_element (ENABLE_NODE, &cli_platform_show_tech_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_tech_list_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_tech_file_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_tech_file_force_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_tech_feature_file_cmd);
  install_element (ENABLE_NODE,
        &cli_platform_show_tech_feature_file_force_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_tech_feature_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_core_dump_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_events_cmd);
  install_element (ENABLE_NODE, &vtysh_diag_dump_cmd);
  install_element (ENABLE_NODE, &vtysh_diag_dump_list_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_vlog_config_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_vlog_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_vlog_config_list_cmd);
  install_element (ENABLE_NODE, &cli_platform_show_vlog_feature_cmd);
  install_element (CONFIG_NODE, &cli_config_vlog_set_cmd);

  /* syslog commands */
  install_element (CONFIG_NODE, &vtysh_config_syslog_basic_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_udp_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_tcp_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_svrt_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_prot_svrt_noport_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_udp_svrt_cmd);
  install_element (CONFIG_NODE, &vtysh_config_syslog_tcp_svrt_cmd);

  install_element (CONFIG_NODE, &no_vtysh_config_syslog_basic_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_udp_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_tcp_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_svrt_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_prot_svrt_noport_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_udp_svrt_cmd);
  install_element (CONFIG_NODE, &no_vtysh_config_syslog_tcp_svrt_cmd);

  retval = install_show_run_config_subcontext(e_vtysh_config_context,
                        e_vtysh_config_context_syslog,
                        &vtysh_config_context_syslog_clientcallback,
                        NULL,NULL);
  if(e_vtysh_ok != retval)
  {
    vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
           "config context unable to add syslog client callback");
    assert(0);
  }
}
