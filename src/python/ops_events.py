#!/usr/bin/env python
from systemd import journal
import sys

def main():
    SEVERITIES = 'event_severities : LOG_EMER, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARN, LOG_NOTICE, LOG_INFO, LOG_DEBUG'
    l = len(sys.argv)
    if l > 3 or l < 3 :
        print 'Usage: ops_events.py <event_string> <event_severity>\n'
        print SEVERITIES
        return
    sev_list = ['LOG_EMER', 'LOG_ALERT', 'LOG_CRIT', 'LOG_ERR', 'LOG_WARN', 'LOG_NOTICE', 'LOG_INFO', 'LOG_DEBUG']
    cmdargs = str(sys.argv)
    msg = sys.argv[1]
    sev = sys.argv[2]
    found = 0
    for i in range(len(sev_list)):
        if sev == sev_list[i]:
            found = 1
            break
    if found is 0:
        print 'Unknown severity level passed'
        print SEVERITIES
        return
    log = 'ops-evt|' + '000000' + '|' + sev + '|' + msg
    journal.send(log, MESSAGE_ID='50c0fa81c2a545ec982a54293f1b1945', PRIORITY=sev)

if __name__ == "__main__": main()
