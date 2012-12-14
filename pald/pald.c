#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <ctype.h>
#include <crypt.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <syslog.h>
#include "pald.h"

#define SOCKPN "127.0.0.1"
#define RECV_DELIMETER ':'
#define PORTNUM 7331

const char SALT_CMD[]="grep -oP '(%s\\:\\$[1-6]\\$.+\\$)' /etc/shadow|awk -F\\$ '{print $3}'\n";
const char SHADOW_CMD[]="grep -oP '(%s\\:.+)' /etc/shadow|awk -F: '{ print $2 }'";

//TODO: Add D-Bus support

int main(int argc, char* argv[])
{
  int crypt_id=getcryptid();

  if(crypt_id>6 || crypt_id<1)
    ragequit("The crypt id found in /etc/shadow is invalid.");

  pid_t pid = 0;
  pid_t sid = 0;

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
      ragequit("Couldn't get session id.\n");
    }
  
  close(STDIN_FILENO);
  close(STDOUT_FILENO);

    //BEGIN DAEMON CODE
  int infd=socket(AF_INET, SOCK_STREAM, 0);
  if (infd < 0) 
    ragequit("can't create socket");

  fcntl(infd, F_SETFD, FD_CLOEXEC);


  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(SOCKPN);
  addr.sin_port = htons(PORTNUM);

  if (bind(infd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    fprintf(stderr,"WARNING: cannot bind to socket %s (%s), exiting\n",
	    SOCKPN, strerror(errno));
    exit(-1);
  }

  if(listen(infd,SOMAXCONN)!=0)
    ragequit("pald failed to listen on open sock\n");
  
  int connection_fd;  
  socklen_t address_length=(socklen_t)sizeof(struct sockaddr);

  //listen on socket, handle in a new process when get new connection
  while((connection_fd = accept(infd, (struct sockaddr *) &addr, &address_length  )) > -1){
    pid_t child = fork();
    if(child<0)

      ragequit("fork");
    else if(child == 0)
      return check_creds(connection_fd, crypt_id);
    else
      close(connection_fd);
  }
  
  close(infd);
  unlink(SOCKPN);
  return 0;
}

void ragequit(const char *msg){
  syslog(LOG_DAEMON|LOG_ERR,"%s", msg);
  exit(-1);
}

unsigned short int isnumber(char *buf){
  while(*buf != '\0' && *buf != RECV_DELIMETER){
    if(!isdigit(*buf))
      return 0;
    buf++;
  }
  return 1;
}

int getcryptid(){ //get hash type from pald.conf (ie: 1: md5, 5: sha-256, 6: sha-512 etc.)
  int cryptid;
  FILE *fin=popen("grep -m 1 -oP '((?<=\\$)([1-6]|(2a))(?=\\$))' /etc/shadow","r");
  fscanf(fin,"%d",&cryptid);
  pclose(fin);
  return cryptid;
}

int mstrcmp(char *s1, char *s2){
  while(*(s1++) == *(s2++))
    if(s1 == '\0' && s2 == '\0')
      return 0;
  return 1;
}
	
int check_creds(int fd, int crypt_id){

  // buffer[261] sets a limit of 255 bytes on user passwords, acceptable for most use cases
  char shadow_credentials[200], shadow_cmd[100], *computed_credentials, crypt_format[44], salt_cmd[100],buffer[261],salty[40],*pass; 
  unsigned int i,GID;
  FILE *pfd;

  struct passwd *user_st;
  
  memset(buffer,'\0',sizeof(buffer)); //set potentially vunlerable strings to \0 to avoid overflows
  memset(salt_cmd,'\0', sizeof(salt_cmd));
  memset(salty,'\0', sizeof(salty));
  memset(crypt_format,'\0', sizeof(crypt_format));
  
  while(1){
    if(recv(fd,buffer,sizeof(buffer)-1,0) > 0){

      pass=strchr(buffer,RECV_DELIMETER); 

      if(pass == NULL)
	exit(-1);
      *pass='\0';
      pass++; //pass now points to password

      if(isnumber(buffer))
	GID=atoi(buffer); 
      else
	ragequit("Received GID is not a number.");

      if((user_st=getpwuid(GID)) == NULL)
	ragequit("Supplied UID/GID not found in /etc/passwd\n");
      
      snprintf(salt_cmd, sizeof(SALT_CMD)-2+sizeof(user_st->pw_name), SALT_CMD, user_st->pw_name); //bash command to return the salt for user user_st->pw_name

      pfd=popen(salt_cmd,"r"); //run salt_cmd

      i=0;
      while((salty[i]=fgetc(pfd)) != '\n')
	i++;
      salty[i]='\0';

      pclose(pfd);

      snprintf(crypt_format, sizeof(salty)+4, "$%d$%s$", crypt_id, salty);
      
      snprintf(shadow_cmd, sizeof(SHADOW_CMD)-2+sizeof(user_st->pw_name), SHADOW_CMD, user_st->pw_name);
      pfd=popen(shadow_cmd,"r");

      i=0;
      while((shadow_credentials[i]=fgetc(pfd)) != '\n')
	i++;
      shadow_credentials[i]='\0';

      computed_credentials=crypt(pass,crypt_format);

      pclose(pfd);
      printf("%s\n", shadow_credentials);
      printf("%s\n",computed_credentials);
      sprintf(buffer,"%d",strcmp(shadow_credentials, computed_credentials));
      buffer[1]='\0';
      if(write(fd, &buffer, sizeof(buffer))<0)
	ragequit("Couldn't return result to sock.\n");
      
    }

  }
  closelog();
  return 0;
}
