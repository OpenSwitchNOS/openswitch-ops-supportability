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

content = []
category = []

# Initialization API for event Log category

def event_log_init(cat):
    global content
    global category
# Search whether category is already initialised
    for i in range(len(category)):
        if cat in category[i]:
# Already initialised, so return.
            return
    with open('/etc/openswitch/supportability/ops_events.yaml', 'r') as f:
        doc = yaml.load(f)
        f.close()
        for txt in range(len(doc['event_definitions'])):
            yaml_cat = doc['event_definitions'][txt]['event_category']
            if yaml_cat == cat:
                mydic = {
                     'event_name': doc['event_definitions'][txt]['event_name'],
                              'event_ID': doc['event_definitions'][txt]['event_ID'],
                              'severity': doc['event_definitions'][txt]['severity'],
                              'description': doc['event_definitions'][txt]['event_description_template'],
                }
# Now add it to global event list
                content.append(mydic)
# Add category to global category list
                category.append(cat)

# Utility API used to replace key with value provided.
def replace_str(keys,desc):
    for j in range(len(keys)):
        key = keys[j][0]
        key = "{" + key + "}"
        value = keys[j][1]
        desc = desc.replace(str(key),str(value))
        return desc

# API to log events from a python daemon
def log_event(name, *arg):
     for i in range(len(content)):
         if name == content[i]['event_name']:
# Found the event in list!
             ev_id = str(content[i]['event_ID'])
             severity = content[i]['severity']
             desc = content[i]['description']
             if len(arg):
                desc = replace_str(arg, desc)
     mesg = 'ops-evt|' + ev_id + '|' + severity + '|' + desc
     journal.send(mesg, MESSAGE_ID='50c0fa81c2a545ec982a54293f1b1945', PRIORITY=severity)
