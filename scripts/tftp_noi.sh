#!/bin/sh
# Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#
#

# This script send file to tftp server

# Usage
# tftp_noi.sh  <tftp server ip> <absolute path of local file to send>
#                               <destination file name>


TIMEOUT=180  #timeout in sec

function timeout_monitor() {
   local   PID=$1
   sleep "$TIMEOUT"
   ps -p $PID  >/dev/null 2>&1
   if [[ $? -eq 0 ]] ; then
       echo "TFTP timeout"
       kill -TERM "$PID"
   fi
}

if [ $# -ne 3 ]; then
    exit 1;
fi

timeout_monitor "$$" &
echo 'copying ...'
tftp $1 <<EOF
bin
put $2 $3
quit
EOF

exit $?
