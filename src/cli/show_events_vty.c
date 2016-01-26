/* System SHOW_EVENTS CLI commands
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
 * File: show_events_vty.c
 *
 * Purpose: To Run Show Events Commands from CLI
 */
#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "show_events_vty.h"
#include "smap.h"
#include "vtysh/memory.h"
#include "openvswitch/vlog.h"
#include "dynamic-string.h"
#include "eventlog.h"
#include "time.h"
#include "systemd/sd-journal.h"

VLOG_DEFINE_THIS_MODULE (vtysh_show_events_cli);

void convert_to_datetime(char *buf,int buf_size,const char *str)
{
   int strl=0;
   char timestring[100];
   char basetime[20];
   char microsec[7];
   time_t t_t;
   struct tm *tmp;

   strl = strlen(str);
   strncpy(basetime,str,(strl-6));
   strncpy(microsec,str+(strl-6),6);
   microsec[6]=0;
   basetime[strl-6]=0;
   t_t = atoi(basetime);
   tmp = localtime(&t_t);
   strftime(timestring,100,"%Y-%m-%d:%H:%M:%S",tmp);
   snprintf(buf,buf_size,"%s.%s",timestring,microsec);

}

const char*
get_value(const char *str)
{
   if(!str)
      return NULL;
   while(*str)
   {
      /*found the split*/
      if(*str=='=')
      {
        if(*(str+1))
        {
          /*value is present*/
            return str+1;
         }
         return NULL;
      }
      str++;
   }

return NULL;
}


/* Function       : cli_show_events
 * Resposibility  : Display Event Logs
 * Return         : 0 on success 1 otherwise
 */
int
cli_show_events(void)
{
  int return_value = 0;
  int events_display_count = 0;
  sd_journal *journal_handle = NULL;

  /* Open Journal File to read Event Logs */
  return_value =  sd_journal_open(&journal_handle,SD_JOURNAL_LOCAL_ONLY);

  if(return_value<0)
  {
    VLOG_ERR("Failed to open journal");
    return CMD_WARNING;
  }

  /* Filter Event Logs from other Journal Logs */
  return_value = sd_journal_add_match(journal_handle,MESSAGE_OPS_EVT_MATCH,0) ;
  if(return_value<0)
  {
    VLOG_ERR("Failed to event logs");
    return CMD_WARNING;
  }

  /* Success, Now print the Header */
  vty_out(vty,"%s---------------------------------------------------%s",
          VTY_NEWLINE,VTY_NEWLINE);
  vty_out(vty,"%s%s","show event logs",VTY_NEWLINE);
  vty_out(vty,"---------------------------------------------------%s",
          VTY_NEWLINE);

  /* For Each Event Log Message  */
  SD_JOURNAL_FOREACH(journal_handle)
  {
    const char *message_data;
    const char *timestamp;
    const char *priority;
    char  tm_buf[80];
    const char *tm;
    const char *msg;
    const char *pri;
   //  time_t raw_time;
    //struct tm ts;
    //char *ts;
    size_t data_length;
    size_t timestamp_length;
    size_t priority_length;

    return_value = sd_journal_get_data(journal_handle, "MESSAGE",
     (const void **)&message_data, &data_length);
    if (return_value < 0)
    {
      VLOG_ERR("Failed to read message field: %s\n", strerror(-return_value));
      continue;
    }

    return_value = sd_journal_get_data(journal_handle
                                      ,"_SOURCE_REALTIME_TIMESTAMP"
                                      ,(const void **)&timestamp
                                      , &timestamp_length);
    if (return_value < 0) {
      VLOG_ERR("Failed to read timestamp field: %s\n", strerror(-return_value));
      continue;
    }

    return_value = sd_journal_get_data(journal_handle, "PRIORITY",
     (const void **)&priority, &priority_length);
    if (return_value < 0) {
      VLOG_ERR("Failed to read priority field: %s\n", strerror(-return_value));
      continue;
    }
    ++events_display_count;

    msg = get_value(message_data);
    pri = get_value(priority);
    tm = get_value(timestamp);

     convert_to_datetime(tm_buf,80,tm);
     //strftime(tm_bf,sizeof(tm_bf),"%a %Y-%m-%d %H:%M:%S %Z",&ts);
    vty_out(vty,"%s|%s|%s%s",tm_buf,pri,msg,VTY_NEWLINE);
  }

  if(!events_display_count)
  {
    vty_out(vty,"No event has been logged in the system%s",VTY_NEWLINE);
  }
  return CMD_SUCCESS;
}


/*
* Action routines for Show Tech CLIs
*/
DEFUN_NOLOCK (cli_platform_show_events,
  cli_platform_show_events_cmd,
  "show events",
  SHOW_STR
  SHOW_EVENTS_STR)
  {
    return cli_show_events();
  }



/*
 * Function           : cli_pre_init
 * Responsibility     : Install the cli nodes
 */

void
cli_pre_init(void)
{
   /* Supportability Doesnt have any new node */

}

/*
 * Function           : cli_post_init
 * Responsibility     : Install the cli action routines
 */
void
cli_post_init()
{
  install_element (ENABLE_NODE, &cli_platform_show_events_cmd);
}
