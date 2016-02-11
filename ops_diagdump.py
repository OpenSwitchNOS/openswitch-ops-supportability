#
# Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#

import ovs.unixctl.server


_diag_daemon_cb_global = None

def init_diag_dump_basic(diag_daemon_cb):
    global _diag_daemon_cb_global
    _diag_daemon_cb_global = diag_daemon_cb
    ovs.unixctl.command_register("dumpdiagbasic", "", 2, 2, diag_basic_unxctl_cb, None)

def diag_basic_unxctl_cb(conn, argv, unused_aux):
    # argv[0] is basic
    # argv[1] is feature name
    if _diag_daemon_cb_global is None:
        buff = ' init_diag_dump_basic internal error.'
    else:
        try:
            buff = _diag_daemon_cb_global(argv)
        except:
            buff  = 'An exception occurred for diagdump handler.'

    conn.reply(buff)
