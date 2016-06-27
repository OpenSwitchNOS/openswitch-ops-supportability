#!/usr/bin/env python
# (c) Copyright [2016] Hewlett Packard Enterprise Development LP
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

from systemd import journal
import yaml
import ovs.vlog
import sys
import threading
import time

content = []
category = []

FAIL = -1
PASS = 1
NOT_FOUND = 0
EV_CATEGORY = "event_category"
EV_DEFINITION = "event_definitions"
EV_NAME = "event_name"
EV_ID = "event_ID"
EV_SEVERITY = "severity"
EV_COUNTER = "counter"
EV_DESCRIPTION = "description"
EV_DESCRIPTION_YAML = "event_description_template"

TRUE = 1
FALSE = 0

# Logging.
vlog = ovs.vlog.Vlog("ops-eventlog")

# Throttling Data Structure
class throttleClass():
    def __init__(self, entry_flag, counter, hashed_value, event_id):
        self.entry_flag   = entry_flag
        self.counter      = counter
        self.hashed_value = hashed_value
        self.event_id     = event_id

throttle_list = []
throttled_global = FALSE

def initialize_throttle_list():
    i = 0
    while (i < 100):
        throttle_list.append(throttleClass(0,0,0,0))
        i = i + 1

# Initialization API for event Log category

# Access Hash table
def timer():
    while True :
        throttled_msg()
        throttleList_reset()
        time.sleep(4)

def throttled_msg():
    global throttled_global
    index = 0
    while (index < 100):
        if (throttled_global == TRUE) :
            if (throttle_list[index].counter < 0):
                mesg = 'ops-evt|'+ str(throttle_list[index].event_id) \
                        + '| Throttled ' +\
                        str(abs(throttle_list[index].counter))\
                        + ' Messages'
                daemon_name = str(sys.argv[0])
                if "/" in daemon_name:
                    daemon_name = daemon_name.split("/")[-1]
                journal.send(mesg,
                        MESSAGE_ID='50c0fa81c2a545ec982a54293f1b1945',
                        SYSLOG_IDENTIFIER=daemon_name)
        index = index + 1
    throttled_global = FALSE

def throttleList_reset():
    i = 0
    while (i != 100):
        throttle_list[i].entry_flag    = 0
        throttle_list[i].counter       = 0
        throttle_list[i].hashed_value  = 0
        throttle_list[i].event_id      = 0
        i = i + 1

    for i in range(len(content)):
        content[i][EV_COUNTER] = 50

def hashed_value(string):
    hashed_value_fun = 31
    hashed_value_fun = (sum(ord(c) for c in string))
    return hashed_value_fun

def access_hash_table(hashed_value, hash_index, event_id, counter):
    global throttled_global
    if (throttle_list[hash_index].entry_flag == FALSE):
        throttle_list[hash_index].entry_flag    = TRUE
        throttle_list[hash_index].counter       = counter
        throttle_list[hash_index].hashed_value  = hashed_value
        throttle_list[hash_index].event_id      = event_id
        return TRUE
    else:
        loop_index = hash_index
        while ( throttle_list[loop_index].entry_flag == TRUE):
            if (throttle_list[loop_index].hashed_value == hashed_value):
                throttle_list[loop_index].counter = \
                                        throttle_list[loop_index].counter -1
                if (throttle_list[loop_index].counter < 0 ):
                    throttled_global = TRUE
                    return FALSE
                else:
                    return TRUE

            loop_index = loop_index + 1
            if (loop_index == 100 ):
                loop_index = 0

            if ( loop_index == hash_index):
                # No empty place found for collide Entry
                throttled_global = TRUE
                return FALSE

        if ( throttle_list[loop_index].entry_flag == FALSE):
            throttle_list[loop_index].entry_flag   = TRUE
            throttle_list[loop_index].event_id     = event_id
            throttle_list[loop_index].hashed_value = hashed_value
            throttle_list[loop_index].counter      = counter
            return TRUE

    return TRUE

def event_log_init(cat):
    global content
    global category
    found = 0
# Search whether category is already initialised
    for i in range(len(category)):
        if cat in category[i]:
# Already initialised, so return.
            return FAIL
    try:
        with open('/etc/openswitch/supportability/ops_events.yaml', 'r') as f:
            doc = yaml.load(f)
            f.close()
            for txt in range(len(doc[EV_DEFINITION])):
                yaml_cat = doc[EV_DEFINITION][txt][EV_CATEGORY]
                if yaml_cat == cat:
                    mydic = {
                        EV_CATEGORY: yaml_cat,
                        EV_NAME: doc[EV_DEFINITION][txt][EV_NAME],
                        EV_ID: doc[EV_DEFINITION][txt][EV_ID],
                        EV_SEVERITY: doc[EV_DEFINITION][txt][EV_SEVERITY],
                        EV_COUNTER: 50,
                        EV_DESCRIPTION: doc[EV_DEFINITION][
                            txt][EV_DESCRIPTION_YAML],
                    }
# Now add it to global event list
                    content.append(mydic)
                    found = 1
            if found is NOT_FOUND:
# This means supplied category name is not there in YAML, so return.
                vlog.err("Event Category not Found")
                return FAIL
            else:
# Add category to global category list
                category.append(cat)
# reset the throttling data structure
                initialize_throttle_list()
                throttleList_reset()
# Starting Timer for throttle events
                t = threading.Thread(target=timer)
                t.daemon = True
                t.start()

    except:
        vlog.err("Event Log Initialization Failed")
        return FAIL

# Utility API used to replace key with value provided.


def replace_str(keys, desc):
    for j in range(len(keys)):
        key = keys[j][0]
        key = "{" + key + "}"
        value = keys[j][1]
        desc = desc.replace(str(key), str(value))
    return desc

# API to log events from a python daemon


def log_event(name, *arg):
    found = 0
    for i in range(len(content)):
        if name == content[i][EV_NAME]:
# Found the event in list!
            ev_id = str(content[i][EV_ID])
            severity = content[i][EV_SEVERITY]
            desc = content[i][EV_DESCRIPTION]
            categ = content[i][EV_CATEGORY]
            if len(arg):
                desc = replace_str(arg, desc)
            found = 1
            break
    if found is NOT_FOUND:
# This means supplied event name is not there in YAML, so return.
        vlog.err("Event not Found")
        return FAIL
    mesg = 'ops-evt|' + ev_id + '|' + severity + '|' + desc
    daemon_name = str(sys.argv[0])
    if "/" in daemon_name:
        daemon_name = daemon_name.split("/")[-1]
    journal.send(mesg,
                 MESSAGE_ID='50c0fa81c2a545ec982a54293f1b1945',
                 PRIORITY=severity,
                 OPS_EVENT_ID=ev_id,
                 OPS_EVENT_CATEGORY=categ,
                 SYSLOG_IDENTIFIER=daemon_name)
    return PASS


def log_event_throttle(counter, name, *arg):
    throttle_flag = FALSE
    found = 0
    for i in range(len(content)):
        if name == content[i][EV_NAME]:
# Found the event in list!
            ev_id = str(content[i][EV_ID])
            severity = content[i][EV_SEVERITY]
            desc = content[i][EV_DESCRIPTION]
            categ = content[i][EV_CATEGORY]
            if len(arg):
                desc = replace_str(arg, desc)
            found = 1
            break
    if found is NOT_FOUND:
# This means supplied event name is not there in YAML, so return.
        vlog.err("Event not Found")
        return FAIL
    mesg = 'ops-evt|' + ev_id + '|' + severity + '|' + desc
# Throttling Logic Start
    content[i][EV_COUNTER] = content[i][EV_COUNTER] - 1

    if (content[i][EV_COUNTER] < 0 ):
        hashed_value_local = hashed_value(mesg)
        hash_index_local = hashed_value_local % 100
        throttle_flag = access_hash_table(hashed_value_local, hash_index_local,\
                content[i][EV_ID], counter)

    if ((content[i][EV_COUNTER] >= 0 ) or (throttle_flag == TRUE) ):
        daemon_name = str(sys.argv[0])
        if "/" in daemon_name:
            daemon_name = daemon_name.split("/")[-1]
        journal.send(mesg,
                     MESSAGE_ID='50c0fa81c2a545ec982a54293f1b1945',
                     PRIORITY=severity,
                     OPS_EVENT_ID=ev_id,
                     OPS_EVENT_CATEGORY=categ,
                     SYSLOG_IDENTIFIER=daemon_name)
        return PASS
    else:
        vlog.err("Event_ID:" + str(ev_id) + " has been Throttled")
        return FAIL
