#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include "gid_definition.h"

void ragequit(const char *msg){
  perror(msg);
  exit(-1);
}

int main(void){

  int sockid=0;

  char *chldsock="/tmp/chldsock";
  size_t chldsockn=sizeof(chldsock);
  //int chldsockn=sprintf(chldsock,"/tmp/sock-%d", sockid++);

  // CREATE A SOCKET TO PASS STDIN, STDOUT
  int chldfd=socket(AF_UNIX, SOCK_STREAM, 0);

  if (chldfd < 0) 
    ragequit("can't create socket");
  
  struct stat st;
    if (stat(chldsock, &st) >= 0) {
	if (!S_ISSOCK(st.st_mode)) {
	    fprintf(stderr, "socket pathname %s exists and is not a socket\n",
		    chldsock);
	    exit(-1);
	}

	unlink(chldsock);
    }

  struct sockaddr_un chldaddr;
  chldaddr.sun_family = AF_UNIX;
  chldaddr.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
  snprintf(chldaddr.sun_path, chldsockn, "%s", chldsock);
  if (bind(chldfd, (struct sockaddr *) &chldaddr, sizeof(chldaddr)) < 0) {
    fprintf(stderr,"WARNING: cannot bind to socket %s (%s), exiting\n",
	    chldsock, strerror(errno));
    exit(-1);
  }
  
  chown(chldsock,1000,1000);
  chmod(chldsock, 0720); 
  
  
  if(listen(chldfd,SOMAXCONN)!=0)
    ragequit("pald failed to listen on open sock");

  int confd;
  socklen_t chldsize=sizeof(struct sockaddr_un);
  printf("made it midway\n");
  while((confd = accept(chldfd, (struct sockaddr *) &chldaddr, &chldsize )) > -1){
    pid_t child = fork();
    if(child<0)
      ragequit("fork");
    else if(child == 0){ //child

      dup2(chldfd,1);
      dup2(chldfd,0);
      close(chldfd);

      execlp("/bin/echo", "got connections");
      /* Code below will never happen unless exec fails */
      perror("Couldn't exec");
      exit(1);
    }
    //parent
    close(chldsock); /* Parent doesn't need socket anymore */
    waitpid(child, &st, 0); /* Wait for child to finish */
  
  }
  return 0;
}
