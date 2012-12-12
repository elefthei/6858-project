# Echo server program
from socket import *
import os, sys, subprocess, struct

def send_int(sock, num):
    sock.send(str(num).zfill(8))

dbg=open("dbginfo","w")

sockid=0

daemonpid=os.fork()
if(daemonpid<0):
    sys.stderr.write('fork1\n')
    sys.exit(-1)

elif(daemonpid==0): #child

    coms = socket(AF_UNIX, SOCK_STREAM)
    try:
        os.remove("/tmp/palsock")
    except OSError:
        pass
    coms.bind("/tmp/palsock")
    coms.listen(1)
    dadconn, dadaddr = coms.accept()
    
    while 1:
    # handle requests
        child=os.fork()
        if(child<0):
            sys.stderr.write('fork2\n')
            sys.exit(-1)
        elif(child==0): #child

            GID=int(dadconn.recv(16))
            UID=int(dadconn.recv(16))

            sockid+=1
            s = socket(AF_UNIX, SOCK_STREAM)
            try:
                os.remove("/tmp/chldsock-%d" % sockid)
            except OSError:
                pass

            s.bind("/tmp/chldsock-%d" %sockid)
            os.chmod("/tmp/chldsock-%d" %sockid, 0777) #changeme


            dadconn.send(str(s.fileno()).zfill(16))
            dadconn.close()

            s.listen(1)
            conn, addr = s.accept()
        
            sys.exit(42);

            if 'OK' not in dadconn.recv(16):
                sys.stderr.write('Didnt get shell fd\n')
                dadconn.send('ERROR Aborting')
                sys.exit(-1)

            while 1:
                data = conn.recv(4096)
                if("exit" in data):
                    conn.send(chr(04))
                    sys.exit(0)

                if not data: continue

                os.setegid(GID)
                os.seteuid(UID)

                op = subprocess.Popen(data, shell=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                if op:
                    output=str(op.stdout.read())
                    conn.sendall(output)

                else:
                    error=str(op.stderr.read())
                    conn.sendall(error)

            conn.close()
            sys.exit(0)

        else:
            os.waitpid(child,0)

else: #parent
    os.waitpid(daemonpid,0)
    sys.exit(0)
