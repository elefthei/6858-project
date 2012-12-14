#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKPN "127.0.0.1"
#define PORTNUM 7331
#define RECV_DELIMETER ':'

int ragequit(const char *msg){
  perror(msg);
  return -1;
}

int pald_request_gid(gid_t GID){

  int sock, rval; 
  struct sockaddr_in server;

  sock = socket(AF_INET, SOCK_STREAM, 0); 
  if (sock < 0)
    return ragequit("error opening stream socket"); 

  server.sin_family = AF_INET; 
  server.sin_addr.s_addr = inet_addr(SOCKPN);
  server.sin_port = htons(PORTNUM);

  if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0) { 
    close(sock); 
    return ragequit("error connecting stream socket"); 
  } 

  char *pass=getpass("Password: ");
  char DATA[265];
  memset(DATA,'\0',265);
  snprintf(DATA,sizeof(DATA), "%d%c%s", GID,RECV_DELIMETER, pass);
  if (write(sock, DATA, sizeof(DATA)) < 0)
    return ragequit("error writing on stream socket"); 
  
  char reply;
  if((rval = read(sock, &reply, 1)) < 0)
    ragequit("error reading stream message");

  close(sock);

  if(reply=='0')
    return 1;
     
  
  return 0;
}
