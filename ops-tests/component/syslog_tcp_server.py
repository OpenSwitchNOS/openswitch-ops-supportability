import socket
import sys
from os import fsync

if len(sys.argv) >= 3:
    TCP_IP = sys.argv[1]
    TCP_PORT = int(sys.argv[2])
elif len(sys.argv) == 2:
    TCP_IP = sys.argv[1]
    TCP_PORT = 1470
else:
    exit()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.bind((TCP_IP, TCP_PORT))
sock.listen(5)
print('listening on %s:%s' % (TCP_IP, TCP_PORT))
while True:
    conn, addr = sock.accept()
    print('New Connection Established')
    try:
        f = open('/tmp/syslog_out.sb', 'w')
        while True:
            data = conn.recv(1024)
            if data:
                f.write(data)
                f.flush()
                fsync(f)
            else:
                break
    finally:
        print('Connection Closed')
        conn.close()
        f.close()
