#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <ctype.h>
#include <crypt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include "pald.h"

#define SOCKPN "/tmp/paldsock"
#define CONF_FILE "pald.conf"
#define RECV_DELIMETER ':'

#define CRED_AUTH_SUCESS "1111"
#define CRED_AUTH_FAIL "0000"

const char SALT_CMD[]="grep -oP '(%s\\:\\$[1-6]\\$.+\\$)' /etc/shadow|awk -F\\$ '{print $3}'\n";
const char SHADOW_CMD[]="grep -oP '(%s\\:.+)' /etc/shadow|awk -F: '{ print $2 }'";

//TODO: Add D-Bus support

int main(int argc, char* argv[])
{
  int crypt_id=getcryptid();

  if(crypt_id>6 || crypt_id<1)
    ragequit("The crypt id found in pald.conf is invalid. Run 'make install again\n");

  pid_t pid = 0;
  pid_t sid = 0;
  FILE *fp= NULL;
  int i = 0;

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
  int infd=socket(AF_UNIX, SOCK_STREAM, 0);
  if (infd < 0) 
    ragequit("can't create socket");

  fcntl(infd, F_SETFD, FD_CLOEXEC);

  struct stat st;
  if (stat(SOCKPN, &st) >= 0) {
    if (!S_ISSOCK(st.st_mode)) {
      fprintf(stderr, "socket pathname %s exists and is not a socket\n",
	      SOCKPN);
      exit(-1);
    }
    
    unlink(SOCKPN);
  }

  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, sizeof(SOCKPN), "%s", SOCKPN);
  if (bind(infd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    fprintf(stderr,"WARNING: cannot bind to socket %s (%s), exiting\n",
	    SOCKPN, strerror(errno));
    exit(-1);
  }

  chmod(SOCKPN, 0777); //change me

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
  perror(msg);
  exit(-1);
}

unsigned short int isnumber(char *buf){
  while(*buf != '\0'){
    buf++;
    if(!isdigit(*buf))
      return 0;
    return 1;
  }
}

int getcryptid(){ //get hash type from pald.conf (ie: 1: md5, 5: sha-256, 6: sha-512 etc.)
  int cryptid;
  FILE *fin=fopen(CONF_FILE,"r");
  fscanf(fin,"%d",&cryptid);
  fclose(fin);
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
  char shadow_credentials[200], ret_val[2],*shadow_creds_pt, shadow_cmd[100], *computed_credentials, crypt_format[44], salt_cmd[100],buffer[261],salty[40],*pass; 
  unsigned int i,GID;
  FILE *pfd;

  struct passwd *user_st;
  
  memset(buffer,'\0',sizeof(buffer)); //set potentially vunlerable strings to \0 to avoid overflows
  memset(salt_cmd,'\0', sizeof(salt_cmd));
  memset(salty,'\0', sizeof(salty));
  memset(crypt_format,'\0', sizeof(crypt_format));
  
  while(1){
    if(recv(fd,buffer,sizeof(buffer)-1,0) > 0){

      pass=strchr(buffer,RECV_DELIMETER); //change this to '\n'

      if(pass == NULL)
	exit(-1);
      *pass='\0';
      pass++; //pass now points to password

      if(isnumber(buffer))
	GID=atoi(buffer);
      else
	ragequit("Received GID is not an int\n");

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

      shadow_creds_pt=&shadow_credentials[0];
      computed_credentials=crypt(pass,crypt_format);

      pclose(pfd);
      printf("%s\n", shadow_credentials);
      printf("%s\n",computed_credentials);
      sprintf(buffer,"%d\0",strcmp(shadow_credentials, computed_credentials));
      if(write(fd, &buffer, sizeof(buffer))<0)
	ragequit("Couldn't return result to sock.\n");
      
    }

  }

  return 0;
}
