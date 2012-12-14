#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SOCKPN "/tmp/paldsock"
#define RECV_DELIMETER ':'

int ragequit(const char *msg){
  perror(msg);
  return -1;
}

int pald_request_gid(gid_t GID){

  int sock, rval; 
  struct sockaddr_un server;

  sock = socket(AF_UNIX, SOCK_STREAM, 0); 
  if (sock < 0)
    return ragequit("error opening stream socket"); 

  server.sun_family = AF_UNIX; 
  strcpy(server.sun_path, SOCKPN);

  if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) { 
    close(sock); 
    return ragequit("error connecting stream socket"); 
  } 

  char *pass=getpass("Password: ");
  char DATA[265];
  snprintf(DATA,sizeof(DATA), "%d%c%s\0", GID,RECV_DELIMETER, pass);
  if (write(sock, DATA, sizeof(DATA)) < 0)
    return ragequit("error writing on stream socket"); 
  
  char reply;
  if((rval = read(sock, reply, 1)) < 0)
    ragequit("error reading stream message");

  close(sock);

  if(reply=='0')
    return 1
     
  
  return 0;
}
