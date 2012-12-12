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
#include <gid_definition.h>

#define sockpn /tmp/paldsock
//TODO: Add D-Bus support

int main(int argc, char* argv[])
{
    pid_t pid = 0;
    pid_t sid = 0;
    FILE *fp= NULL;
    int i = 0;
    long long sockid=0;

    pid = fork();// fork a new child process

    if (pid < 0)
      ragequit("fork failed!\n");

    if (pid > 0)// its the parent process
    {
      printf("pid of child process %d \n", pid);
      exit(0); //terminate the parent process succesfully
    }

    umask(S_IROTH|S_IWOTH); //unmasking the file mode

    sid = setsid(); //set new session
    if(sid < 0)
    {
        exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    //BEGIN DAEMON CODE
    int infd=socket(AF_UNIX, SOCK_STREAM, 0);
    if (infd < 0) 
	ragequit("can't create socket");

    fcntl(infd, F_SETFD, FD_CLOEXEC);

    struct stat st;
    if (stat(sockpn, &st) >= 0) {
	if (!S_ISSOCK(st.st_mode)) {
	    fprintf(stderr, "socket pathname %s exists and is not a socket\n",
		    sockpn);
	    exit(-1);
	}

	unlink(sockpn);
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(sockpn), "%s", sockpn);
    if (bind(infd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      fprintf(stderr,"WARNING: cannot bind to socket %s (%s), exiting\n",
	     sockpn, strerror(errno));
      exit(-1);
    }

    chmod(sockpn, 0772); //onwer&group: RWX, others: R
    if(listen(infd,SOMAXCONN)!=0)
      ragequit('pald failed to listen on open sock');

    int connection_fd;  
    socklen_t address_length;

  //listen on socket, handle in a new process when get new connection
  while((connection_fd = accept(ss_fd, (struct sockaddr *) &addr, )) > -1){
    child = fork();
    if(child<0)
      ragequit('fork');
    else if(child == 0)
      return connection_handler(connection_fd);


   /* still inside server process */
    close(connection_fd);
  }

  
  close(ss_fd);
  unlink(sockpn);
  return 0;
}

void ragequit(const char *msg){
  perror(msg);
  exit(-1);
}

int connection_handler(int fd){

  int count, *buffer;  
  size_t countlen=read(fd, &count, sizeof(count));
  if(countlen<=0)
    ragequit("connection_hanlder: read 1\n");

  buffer=(char*)malloc(count);

  countlen=read(fd, buffer, count);  
  if(countlen<=0)
    ragequit("connection_hanlder: read 2\n");

  
  char *chldsock;
  int chldsockn=sprintf(chldsock,"/tmp/sock-%d", sockid++);

  // CREATE A SOCKET TO PASS STDIN, STDOUT
  int chldfd=socket(AF_UNIX, SOCK_STREAM, 0);

  if (chldfd < 0) 
    ragequit("can't create socket");
  
  struct sockaddr_un chldaddr;
  chldaddr.sun_family = AF_UNIX;
  snprintf(chldaddr.sun_path, chldsockn, "%s", chldsock);
  if (bind(chldfd, (struct sockaddr *) &chldaddr, sizeof(chldaddr)) < 0) {
    fprintf(stderr,"WARNING: cannot bind to socket %s (%s), exiting\n",
	    chldsock, strerror(errno));
    exit(-1);
  }
  
  chown(chldsock,1000,1000);
  chmod(chldsock, 0720); 
  
  
  if(listen(chldfd,SOMAXCONN)!=0)
    ragequit('pald failed to listen on open sock');

  dup2(chldfd,1);
  dup2(chldfd,0);
  close(chldfd);

  free(buffer);
  close(fd);

  execlp("/bin/sh", NULL);
  /* Code below will never happen unless exec fails */
  perror("Couldn't exec");
  exit(1);

  
  return 0;

}

  
    //CONTINUE HERE
    

    return (0);
}
