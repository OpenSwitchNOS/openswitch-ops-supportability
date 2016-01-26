/* SHOW_EVENTS CLI commands.
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
 * File: show_events_vty.h
 *
 * Purpose: To Run Show Events command from CLI.
 */

#ifndef _SHOW_EVENTS_VTY_H
#define _SHOW_EVENTS_VTY_H

void cli_pre_init(void);
void cli_post_init(void);

#define SHOW_EVENTS_STR              "Display event logs\n"
#define MESSAGE_OPS_EVT_MATCH "MESSAGE_ID=50c0fa81c2a545ec982a54293f1b1945"

#define BUF_SIZE 100
#define BASE_SIZE 20
#define MICRO_SIZE 7
#define MIN_SIZE 6

/*Function  : convert_to_datetime
 * Responsibility : to convert the real timestamp in to unix timestamp date-time
 * return : none
 */

void
convert_to_datetime(char *buf,int buf_size,const char *str)
{
   int strl=0;
   char timestring[BUF_SIZE];
   char basetime[BASE_SIZE];
   char microsec[MICRO_SIZE];
   time_t t_t;
   struct tm *tmp;
   strl = strlen(str);
   if(strl>=MIN_SIZE)
   {
   strncpy(basetime,str,(strl-MIN_SIZE));
   strncpy(microsec,str+(strl-MIN_SIZE),MIN_SIZE);
   microsec[MIN_SIZE]=0;
   basetime[strl-MIN_SIZE]=0;
   t_t = atoi(basetime);
   tmp = localtime(&t_t);
   strftime(timestring,100,"%Y-%m-%d:%H:%M:%S",tmp);
   snprintf(buf,buf_size,"%s.%s",timestring,microsec);
   }
}

#endif //_SHOW_EVENTS_VTY_H
