import socket, sys, struct, subprocess


if(len(sys.argv) != 2):
    sys.stderr.write('Invalid arguments: %s <new gid> \n' % sys.argv[0])
    sys.exit(-1)

GID=int(sys.argv[1])

pals = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
pals.connect("/tmp/palsock")

pals.send(str(GID).zfill(16))
pals.send(str(getuid()).zfill(16))

fd=int(pals.recv(16))
print "fd= %d" % fd

s=socket.fromfd(fd,socket.AF_UNIX, socket.SOCK_STREAM)

# pals.send('ERROR')
# sys.stderr.write('Getting socket from fd %s\n')
# sys.exit(-1)

#print 'Got it'
pals.send(str(UID).zfill(16))

while 1:
    sys.stdout.write('$ ')
    command=sys.stdin.readline()
    s.send(command)
    data = s.recv(4096)
    if(chr(04) in data):
        s.close()
        sys.exit(0)
    sys.stdout.write(data)

