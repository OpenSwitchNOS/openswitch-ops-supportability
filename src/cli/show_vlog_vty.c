/* SHOW VLOG LIST CLI commands
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
 * File: show_vlog_vty.c
 *
 * Purpose: To Run Show Events Commands from CLI
 */

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "smap.h"
#include "vtysh/memory.h"
#include "openvswitch/vlog.h"
#include <yaml.h>
#include "jsonrpc.h"
#include "feature_mapping.h"
#include <string.h>
#include "show_vlog_vty.h"

#define LIST_ARGC 0
#define ADD_TOK 2
#define SET_ARGC 3
#define MAX_SIZE 100
#define LIST "vlog/list"
#define SET  "vlog/set"
#define FEATURE "feature"
#define DAEMON "daemon"
#define FEATURE_REQUEST 1
#define DAEMON_REQUEST  2
#define SHOW_VLOG_REQUEST 3
#define SET_REQUEST 4

VLOG_DEFINE_THIS_MODULE(vtysh_vloglist_cli);

static int
vtysh_vlog_interface_daemon(char *feature,char *daemon ,char **cmd_type ,
      int cmd_argc , int request,struct vty *vty);

static struct jsonrpc *
vtysh_vlog_connect_to_target(const char *target);

int
strcmp_with_nullcheck( const char *, const char *);

static struct feature *feature_head =NULL;

/*flag to check before parsing yaml file */
static int initialized =0;

/*
 * Function       : vtysh_vlog_connect_to_target
 * Responsibility : populates jsonrpc client structure for a daemon
 * Parameters     : target  - daemon name
 * Returns        : jsonrpc client on sucess
 *                   NULL on Failure
 */

 static struct jsonrpc *
 vtysh_vlog_connect_to_target(const char *target)
 {
   struct jsonrpc *client=NULL;
   char *socket_name=NULL;
   int error=0;
   char * rundir = NULL;
   char *pidfile_name = NULL;
   pid_t pid = -1;

    if (!target) {
        VLOG_ERR("target is null");
        return NULL;
    }

    rundir = (char*)ovs_rundir();

    if(!rundir) {
         VLOG_ERR("rundir is null");
         return NULL;
    }

    if (target[0] != '/') {
       pidfile_name = xasprintf("%s/%s.pid", rundir ,target);
       if (!pidfile_name) {
          VLOG_ERR("pidfile_name is null");
          return NULL;
       }

      pid = read_pidfile(pidfile_name);
       if (pid < 0) {
          VLOG_ERR("cannot read pidfile :%s", pidfile_name);
           free(pidfile_name);
            return NULL;
       }

        free(pidfile_name);
        socket_name = xasprintf("%s/%s.%ld.ctl", rundir , target,
              (long int) pid);
         if (!socket_name) {
            VLOG_ERR("socket_name is null");
            return NULL;
         }

    }
    else {
        socket_name = xstrdup(target);
         if (!socket_name) {
            VLOG_ERR("socket_name is null, target:%s",target);
             return NULL;
         }
    }

    error = unixctl_client_create(socket_name, &client);
    if (error) {
       VLOG_ERR("cannot connect to %s,error=%d", socket_name,error);
    }
    free(socket_name);
    return client;
 }

/*
 * Function       : vtysh_vlog_interface_daemon
 * Responsibility : send request to daemon using unixctl and get the result
 *                : and print on the console.
 * Parameters     : feature
 *                : daemon
 *                : cmd_type
 *                : cmd_argc
 *                : vty
 * Returns        : 0 on success and non-zero on failure
 */

static int
vtysh_vlog_interface_daemon(char *feature,char *daemon ,char **cmd_type,
            int cmd_argc ,int request,struct vty *vty)
{
   struct jsonrpc *client = NULL;
   char *cmd_result = NULL;
   char *cmd_error = NULL;
   int rc=0;
   char **cmd_argv = cmd_type;
   int cmd_argcount = cmd_argc;
   char vlog_str[MAX_SIZE];
   int  opt=1;

   if(!(daemon && cmd_type)) {
      VLOG_ERR("invalid paramter daemon or command");
      return CMD_WARNING;
   }

   client = vtysh_vlog_connect_to_target(daemon);
   if(!client) {
      VLOG_ERR("%s transaction error. client is NULL",daemon);
      vty_out(vty,"failed to connect daemon %s %s",daemon,VTY_NEWLINE);
      return CMD_WARNING;
   }
   if(!strcmp_with_nullcheck(*cmd_type,LIST)) {
      strncpy(vlog_str,LIST,sizeof(vlog_str));
      } else if(!strcmp_with_nullcheck(*(cmd_type+1),SET)) {
            strncpy(vlog_str,SET,sizeof(vlog_str));
            opt = opt +1;
            cmd_argcount = cmd_argc - (opt);
            cmd_argv = cmd_argcount ? cmd_type + opt : NULL;

        }

      rc = unixctl_client_transact(client,vlog_str,cmd_argcount,cmd_argv,
            &cmd_result,&cmd_error);

 /*
  * unixctl_client_transact() api failure case
  *  check cmd_error and rc value.
  * Nonzero rc failure case
 */
      if(rc) {
          VLOG_ERR("%s: transaction error:%s , rc =%d", daemon ,
                (cmd_error?cmd_error:"error") , rc);

          jsonrpc_close(client);
          free(cmd_result);
          free(cmd_error);
      }

      if(cmd_error) {
         VLOG_ERR("%s: server returned error:cmd_error str:%s,rc =%d",
                                 daemon ,cmd_error, rc);
         jsonrpc_close(client);
          free(cmd_result);
          free(cmd_error);
         return CMD_WARNING;
      }

      switch(request)
      {
         case 1:  /*feature result*/
                   vty_out(vty,"%-17.17s   %-17.17s%s",feature,(cmd_result+115),VTY_NEWLINE);
                   break;

         case 2: /*daemon result*/
                  vty_out(vty,"%-17.17s %-17.17s%s",daemon,(cmd_result+115),VTY_NEWLINE);
                  break;

         case 3: /*show vlog result*/
                  vty_out(vty,"%-15.15s %-13.13s %-18.18s%s",feature,daemon,(cmd_result+115),VTY_NEWLINE);
                  break;

         case 4:  break; //SET REQUEST only for configuration changes by using show to get the changes

         default: break;

      }
       jsonrpc_close(client);
       free(cmd_result);
       free(cmd_error);

   return CMD_SUCCESS;
}


/* Function       :  cli_showvlog_feature
 * Responsibility :  Displays show vlog feature
 * Return         :  0 on Success 1 otherwise
 */

int
cli_showvlog_feature(const char *argv0, const char *argv1)
{

   static int rc = 0;
   int fun_argc = LIST_ARGC;
   char *fun_argv = (char*)malloc(20);
   struct feature *iter = feature_head;
   struct daemon *iter_daemon = NULL;
   int request;
   if( argv0 == NULL ||argv1 == NULL) {
      free(fun_argv);
      return CMD_WARNING;
   } else if(!strcmp_with_nullcheck(argv0,FEATURE)) {
      request = FEATURE_REQUEST;
      strncpy(fun_argv,LIST,20);

   if(!initialized) {
      feature_head = get_feature_mapping();
      if(feature_head == NULL){
         vty_out(vty,"Failed to Map Feature to Daemon%s",VTY_NEWLINE);
         free(fun_argv);
         return CMD_WARNING;
      }
   }

 /* traverse linked list to find feature */
for (iter=feature_head ; iter && strcmp_with_nullcheck(iter->name,argv1);
         iter = iter->next);
      if(iter) {
         VLOG_DBG("feature:%s",iter->name);
         vty_out(vty,"========================================%s",VTY_NEWLINE);
         vty_out(vty,"Feature               Syslog     File%s",VTY_NEWLINE);
         vty_out(vty,"========================================%s",VTY_NEWLINE);
         iter_daemon = iter->p_daemon;
         /*traverse all daemons*/
         while(iter_daemon) {

            rc = vtysh_vlog_interface_daemon(iter->name,iter_daemon->name,&fun_argv,
                     fun_argc,request,vty);
            if (!rc) {
                VLOG_DBG("daemon :%s , rc:%d",iter_daemon->name,rc);
             }
            else{
             VLOG_DBG("daemon :%s , rc:%d",iter_daemon->name,rc);
             }
            iter_daemon = iter_daemon->next;
          }
      }else {
         VLOG_ERR("%s feature is not present",argv1);
         vty_out(vty,"%s feature is not present %s",argv1, VTY_NEWLINE);
         free(fun_argv);
         return CMD_WARNING;
       }
   } else if(!strcmp_with_nullcheck(argv0,DAEMON)){
         request = DAEMON_REQUEST;
         strncpy(fun_argv,LIST,20);
         vty_out(vty,"======================================%s",VTY_NEWLINE);
         vty_out(vty,"Daemon              Syslog     File%s",VTY_NEWLINE);
         vty_out(vty,"======================================%s",VTY_NEWLINE);

         rc = vtysh_vlog_interface_daemon(NULL,(char *)argv1,&fun_argv,fun_argc,request,vty);

         if (!rc) {
             VLOG_DBG("daemon :%s , rc:%d",argv1,rc);
         } else {
           vty_out(vty,"%s daemon is not present %s",argv1, VTY_NEWLINE);
           vty_out(vty,"Not able to communicate with Daemon %s%s",argv1,VTY_NEWLINE);
            free(fun_argv);
           return CMD_WARNING;
         }

      free(fun_argv);
     return CMD_SUCCESS;
   }
      free(fun_argv);
   return CMD_SUCCESS;
}

/* Function       :  cli_show_vlog_list
 * Responsibility :  Display features list
 * Return         :  0 on Success 1 otherwise
 */

int
cli_show_vlog_list(void)
{

if(!initialized) {
      feature_head = get_feature_mapping();
      if(feature_head == NULL){
         vty_out(vty,"Failed to Map Feature to Daemon%s",VTY_NEWLINE);
         return CMD_WARNING;
      }
   }
   struct feature *iter = feature_head;
   vty_out(vty,"=============================================%s",VTY_NEWLINE);
   vty_out(vty,"Features          Description%s",VTY_NEWLINE);
   vty_out(vty,"=============================================%s",VTY_NEWLINE);
    while(iter != NULL) {
       vty_out(vty,"%-17.17s %-35.35s %s",iter->name,iter->desc,VTY_NEWLINE);
     iter =iter->next;
    }
   return CMD_SUCCESS;
}


/* Function       :  cli_config_vlog_set
 * Responsibility :  configure feature loglevel
 * Return         :  0 on Success 1 otherwise
 */

int
cli_config_vlog_set(const char* type,
      const char *fd_name ,const char *destination,
      const char *level)
{
   static int rc =0;
   int len = 0;
   int fun_argc = SET_ARGC;
   char *fun_argv[SET_ARGC];
   char *name = NULL;
   int request = SET_REQUEST;
   struct feature *iter = feature_head;
   struct daemon *iter_daemon = NULL;

   if( fd_name == NULL) {
      return CMD_WARNING;
   } else {
      len = strlen(destination)+ strlen(level) + ADD_TOK;
      name = (char*)calloc(len,sizeof(char));
      strcat(name,destination);
      strcat(name,":");
      strcat(name,level);

      fun_argv[0] = (char *)fd_name;
      fun_argv[1] = SET;
      fun_argv[2] = name;
   }

   if(strcmp_with_nullcheck(type,FEATURE) == 0)
   {
      if(!initialized) {
         feature_head = get_feature_mapping();
         if(feature_head == NULL){
            vty_out(vty,"Failed to Map Feature to Daemon%s",VTY_NEWLINE);
            free(name);
            return CMD_WARNING;
         }
      }
      /*traverse linked list to find feature*/
      for (iter=feature_head ; iter && strcmp_with_nullcheck(iter->name,fd_name);
            iter = iter->next);

      if(iter) {
         VLOG_DBG("feature:%s",iter->name);
         iter_daemon = iter->p_daemon;
         while(iter_daemon) {
            rc = vtysh_vlog_interface_daemon(iter->name,iter_daemon->name,fun_argv,
                  fun_argc, request,vty);
            if (!rc) {
               VLOG_DBG("daemon :%s , rc:%d",iter_daemon->name,rc);
            }
            iter_daemon = iter_daemon->next;
         }
      }
      else {
         VLOG_ERR("%s feature is not present",fd_name);
         vty_out(vty,"%s feature is not present %s",fd_name, VTY_NEWLINE);
         free(name);
         return CMD_WARNING;
      }
   }
   else
   {
      /* Daemon Name is directly given */
      rc = vtysh_vlog_interface_daemon(NULL,(char *)fd_name,fun_argv,
            fun_argc,request,vty);
      if (!rc) {
         VLOG_DBG("daemon :%s , rc:%d",fd_name,rc);
      }
      else
      {
         vty_out(vty,"Not able to communicate with Daemon %s%s",fd_name,VTY_NEWLINE);
         free(name);
         return CMD_WARNING;
      }

   }
   free(name);
   return CMD_SUCCESS;
}

/*CLI to configure the log settings of FILE or SYSLOG */

DEFUN_NOLOCK (cli_config_set_vlog,
      cli_config_vlog_set_cmd,
      "vlog (feature|daemon) NAME (SYSLOG | FILE | ALL) (EMER | ERR | WARN | INFO | DBG)",
      VLOG_SET_STR
      VLOG_CONFIG_FEATURE)
{
   return cli_config_vlog_set(argv[0],argv[1],argv[2],argv[3]);
}

/*Action routine for show vlog features list*/

DEFUN_NOLOCK (cli_platform_show_vlog_list,
   cli_platform_show_vlog_list_cmd,
   "show vlog list",
   SHOW_STR
   VLOG_LIST_STR
   VLOG_LIST_FEATURE)
{
   return cli_show_vlog_list();
}

/* Function       :  cli_showvlog
 * Responsibility :  Display all features loglevels of
 *                   file & console destinations
 * Return         :  0 on Success 1 otherwise
 */
int
cli_showvlog()
{
   char *fun_argv= (char *)malloc(20);
   static int rc = 0;
   int request = SHOW_VLOG_REQUEST;
   struct feature *iter = feature_head;
   struct daemon *iter_daemon = NULL;
   if(!initialized) {
      feature_head = get_feature_mapping();
      if(feature_head == NULL){
         vty_out(vty,"Failed to Map Feature to Daemon%s",VTY_NEWLINE);
         free(fun_argv);
         return CMD_WARNING;
      }
   }
   vty_out(vty,"=================================================%s",VTY_NEWLINE);
   vty_out(vty,"Feature         Daemon          Syslog     File%s",VTY_NEWLINE);
   vty_out(vty,"=================================================%s",VTY_NEWLINE);
   for(iter = feature_head ; iter != NULL ; iter = iter->next)
   {
      if(iter) {

      strncpy(fun_argv,LIST,20);
         iter_daemon = iter->p_daemon;
         while(iter_daemon) {

            rc = vtysh_vlog_interface_daemon(iter->name,iter_daemon->name,&fun_argv,0,request,vty);
             if (!rc) {
                VLOG_DBG("daemon :%s , rc:%d",iter_daemon->name,rc);
              }
               iter_daemon = iter_daemon->next;
            }
      }else{
               vty_out(vty,"%s feature is not present %s",iter->name, VTY_NEWLINE);
               free(fun_argv);
               return CMD_WARNING;
          }
   }
   free(fun_argv);
   return CMD_SUCCESS;
}

/*Action routine for show vlog features,log levels */

DEFUN_NOLOCK (cli_platform_show_vlog,
   cli_platform_show_vlog_cmd,
   "show vlog",
    SHOW_STR
    VLOG_LIST_FEATURE)
{

    return cli_showvlog();

}

/*Action routine for show vlog feature*/

DEFUN_NOLOCK (cli_platform_showvlog_feature_list,
   cli_platform_showvlog_feature_cmd,
   "show vlog (feature | daemon) NAME",
   SHOW_STR
   VLOG_LIST_STR
   VLOG_LIST_FEATURE)
{
   return cli_showvlog_feature(argv[0],argv[1]);
}
