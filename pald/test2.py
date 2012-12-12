# Echo server program
from socket import *
import os, sys, subprocess


class MySocket(socket):
    def __init__(self, *args):
        socket.__init__(self, *args)
	         
    def write(self, text):
        return self.send(text)
	     
    def readlines(self):
        return self.recv(2048)
	     
    def read(self):
        return self.recv(1024)


s = MySocket(AF_UNIX, SOCK_STREAM)
try:
    os.remove("/tmp/socketname")
except OSError:
    pass
s.bind("/tmp/socketname")
s.listen(1)
conn, addr = s.accept()
while 1:
    data = conn.recv(4096)
    if not data: continue
    print data

    op = subprocess.Popen(data, shell=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    if op:
        output=str(op.stdout.read())
        conn.sendall(output)

    else:
        error=str(op.stderr.read())
        conn.sendall(error)

conn.close()
