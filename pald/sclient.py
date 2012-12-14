import socket, sys, struct, os


if(len(sys.argv) != 2):
    sys.stderr.write('Invalid arguments: %s <new gid> \n' % sys.argv[0])
    sys.exit(-1)

GID=int(sys.argv[1])

pals = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
pals.connect("/tmp/paldsock")

while 1:
    sys.stdout.write(': ')
    command=sys.stdin.readline().rstrip('\n')
    pals.send(command)
    data = pals.recv(1)
    if(chr(04) in data):
        pals.close()
        sys.exit(0)
    sys.stdout.write(data)

