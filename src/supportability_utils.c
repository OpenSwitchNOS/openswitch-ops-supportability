/*
 *  (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *  Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License. You may obtain
 *  a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

/************************************************************************//**
 * @ingroup ops-supportability
 *
 * @file
 * Source file for the supportability common utils
 ***************************************************************************/

#include "supportability_utils.h"
#include "supportability_vty.h"

VLOG_DEFINE_THIS_MODULE (supportability_utils_debug);

/* Function        : strncmp_with_nullcheck
 * Responsibility  : Ensure arguments are not null before calling strncmp
 * Return          : -1 if arguments are null otherwise return value from
 *                    strncmp
 */
int
strncmp_with_nullcheck( const char * str1, const char * str2, size_t num )
{
  if(str1 == NULL || str2 == NULL)
    return -1;
  return strncmp(str1,str2,num);
}





/* Function        : strcmp_with_nullcheck
 * Responsibility  : Ensure arguments are not null before calling strcmp
 * Return          : -1 if arguments are null otherwise return value from
 *                   strcmp
 */
int
strcmp_with_nullcheck( const char * str1, const char * str2 )
{
  if(str1 == NULL || str2 == NULL)
    return -1;
  return strcmp(str1,str2);
}



/* Function        : strdup_with_nullcheck
 * Responsibility  : Ensure arguments are not null before calling strdup
 * Return          : null if argument is null otherwise return value form strdump
 */
char *
strdup_with_nullcheck( const char * str1)
{
  if(str1 == NULL)
    return NULL;
  return strdup(str1);
}


/* Helper function to trim white space around the core dump folder location */
char *
trim_white_space(char *string)
{
   char *endptr;
   char *beginptr = string;

   if(string == NULL)
   {
      return NULL;
   }
   /* Remove the white spaces at the beginning */
   while (isspace( (unsigned char)(*beginptr)))
   {
      beginptr++;
   }
   /* if the string contains only whitespace character */
   if(*beginptr == 0)
   {
      return beginptr;
   }

   /* Move the terminating null character next to the last non
      whitespace character */
   endptr = beginptr + strlen(beginptr) - 1;
   while(endptr > beginptr && (isspace( (unsigned char)(*endptr))) )
   {
      endptr--;
   }

   /* endptr points to the last valid entry, now the next entry should be
      terminating null character */
   *(endptr+1) = 0;
   return beginptr;
}

/* Helper function to compile the regex pattern */
int
compile_corefile_pattern (regex_t * regexst, const char * pattern)
{
    int status = regcomp (regexst, pattern, REG_EXTENDED|REG_NEWLINE);
    return status;
}

/* Function       : sev_level
 * Responsibility : To convert severity strings to values
 * return         : -1 if failed, otherwise severity value
 */
int
sev_level(char *arg)
{
    const char *sev[] = {"emer","alert","crit","err","warn","notice","info","debug"};
    int i = 0, found = 0;
    for(i = 0; i < MAX_SEVS; i++)
    {
        if(!strcmp_with_nullcheck(arg, sev[i])) {
            found = 1;
            break;
        }
    }
    if(found) {
        return i;
    }
    return -1;
}

/* Function       : get_values
 * Responsibility : read only values from keys
 * return         : return value
 */

const char*
get_value(const char *str)
{
   if(!str) {
      return NULL;
   }
   while(*str!='\0')
   {
      /*found the split*/
      if(*str == '=')  {
          if(*(str+1))  {
             /*value is present*/
               return str+1;
           }
          return NULL;
      }
      str++;
   }
return NULL;
}

/* Function  : get_yaml_tokens
 * Responsibility : to read yaml file tokens.
 */
char*
get_yaml_tokens(yaml_parser_t *parser,  yaml_event_t **tok, FILE *fh)
{
    char *key;
    yaml_event_t *token = *tok;
    if(fh == NULL) {
        return NULL;
    }
    if(!yaml_parser_parse(parser, token)) {
        return NULL;
    }

    if(token == NULL) {
        return NULL;
    }
    while(token->type!= YAML_STREAM_END_EVENT)
    {
        switch(token->type)
        {
            case YAML_SCALAR_EVENT:
                key = (char*)token->data.scalar.value;
                return key;

            default: break;
        };
        if(token->type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(token);
        }
        if(!yaml_parser_parse(parser, token)) {
            return NULL;
        }
    }
    return NULL;
}

/* Function  : strupr
 * Responsibility : to convert from lower case to upper.
 * return : NULL on failure, otherwise string
 */
char*
strnupr(char *str, int size)
{
    int count = 0;
    unsigned char *p = (unsigned char *)str;
    if(str == NULL) {
        return NULL;
    }

    while (*p) {
        *p = toupper(*p);
        p++;
        count++;
        if(count == size) {
            break;
        }
    }

    return str;
}

/*
 * Function           : strlwr
 * Responsibility     : To convert string from upper to lower case
 */
char*
strnlwr(char *str, int size)
{
    int count = 0;
    unsigned char *p = (unsigned char *)str;
    if(str == NULL) {
        return NULL;
    }
    while (*p) {
        *p = tolower(*p);
        p++;
        count++;
        if(count == size) {
            break;
        }
    }
    return str;
}



/*
 * Function       : validate_cli_args
 * Responsibility : validates given cli argument with regular expression.
 * Parameters
 *                : arg - argument passed in cli
 *                : regex - regular expression to validate user input
 *
 * Returns        : 0 on success
 */

int
validate_cli_args(const char * arg , const char * regex)
{
    regex_t r;
    int rc = 0;
    const int n_matches = 10;
    regmatch_t m[n_matches];

    if (!( arg && regex ) )
        return 1;

    rc = regcomp(&r, regex , REG_EXTENDED|REG_NEWLINE);
    if ( rc )  {
        regfree (&r);
        return rc;
    }

    rc = regexec (&r,arg,n_matches, m, 0);
    regfree (&r);
    return rc;
}

/*
 * Function       : connect_to_daemon
 * Responsibility : populates jsonrpc client structure for a daemon
 * Parameters     : daemon - daemon name
 * Returns        : jsonrpc client on success
 *                  NULL on failure
 *
 */

struct jsonrpc*
connect_to_daemon(const char *daemon) {
    struct jsonrpc *client=NULL;
    char *socket_name=NULL;
    int error=0;
    char * rundir = NULL;
    pid_t pid=-1;

    if (!daemon) {
        VLOG_ERR("Daemon is NULL");
        return NULL;
    }

    rundir = (char*) ovs_rundir();
    if (!rundir) {
        VLOG_ERR("Rundir is NULL");
        return NULL;
    }

    pid = proc_daemon_pid(daemon);
    if (pid < 0) {
        VLOG_ERR("PID not found for daemon:%s", daemon);
        return NULL;
    }
    socket_name = xasprintf("%s/%s.%ld.ctl", rundir , daemon,
            (long int) pid);
    if (!socket_name) {
        VLOG_ERR("Socket name is NULL");
        return NULL;
    }

    error = unixctl_client_create(socket_name, &client);
    if (error) {
        VLOG_ERR("Cannot connect to %s,error=%d", socket_name,error);
        free(socket_name);
        return NULL;
    }
    free(socket_name);

    return client;
}

/*
 * Function       : proc_daemon_pid
 * Responsibility : provides pid for a given daemon name
 * Parameters
 *                : name - regular expression to validate user input
 *
 * Returns        : pid value ( > 0 ) of given daemon on success
 *                  Negative value on failure
 */

pid_t proc_daemon_pid(const char* daemon_name)
{
    DIR* dir;
    struct dirent* ent;
    long  pid , lpid;
    char buf[PROC_FILE_MAX_LEN] = {0,};
    char pname[DAEMON_NAME_MAX_LEN] = {0,};
    char state;
    FILE *fp=NULL;
    char err_buf[MAX_STR_BUFF_LEN]={0};

    if ( daemon_name == NULL )
    {
        VLOG_ERR("Invalid parameter : daemon name");
        return -1;
    }

    if (!(dir = opendir("/proc")))
    {
        strerror_r (errno,err_buf,sizeof(err_buf));
        VLOG_ERR("Unable to open /proc file system, reason:%s",err_buf);
        return -1;
    }

    while((ent = readdir(dir)) != NULL)
    {
        lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");
        if (fp)
        {
            if ( (fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 )
            {
                fclose(fp);
                closedir(dir);
                bzero(err_buf, sizeof(err_buf));
                strerror_r (errno,err_buf,sizeof(err_buf));
                VLOG_ERR("Failed to read file:%s, reason%s:",buf,err_buf);
                return -1;
            }

            fclose(fp);
            if (!strcmp_with_nullcheck(pname, daemon_name))
            {
                closedir(dir);
                return (pid_t)lpid; /* success case */
            }
        }
    } /* while */

    closedir(dir);
    VLOG_ERR("Failed to get pid for daemon:%s, reason:%s",daemon_name,
            "daemon is not found in /proc file system" );
    return -1;
}
