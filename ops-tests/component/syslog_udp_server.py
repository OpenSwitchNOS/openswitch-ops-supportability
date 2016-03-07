import socket
import sys
from os import fsync

if len(sys.argv) >= 3:
    UDP_IP = sys.argv[1]
    UDP_PORT = int(sys.argv[2])
elif len(sys.argv) == 2:
    UDP_IP = sys.argv[1]
    UDP_PORT = 11514
else:
    exit()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind((UDP_IP, UDP_PORT))

f = open('/tmp/syslog_out.sb', 'w')
i = 0
while i < 50:
    data, addr = sock.recvfrom(1024)
    f.write(data)
    f.flush()
    fsync(f)
    i = i + 1
f.close()
