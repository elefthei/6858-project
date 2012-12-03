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

#define sockpn /tmp/grpd.sock
//TODO: Add D-Bus support

int main(int argc, char* argv[])
{
    pid_t pid = 0;
    pid_t sid = 0;
    FILE *fp= NULL;
    int i = 0;
    pid = fork();// fork a new child process

    if (pid < 0)
    {
        printf("fork failed!\n");
        exit(1);
    }

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
    close(STDERR_FILENO);

    //BEGIN DAEMON CODE
    int infd=socket(AF_UNIX, SOCK_STREAM, 0);
    if (srvfd < 0) {
	perror("can't create socket");
	exit(-1);
    }

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
    snprintf(&addr.sun_path[0], sizeof(addr.sun_path), "%s", sockpn);
    if (bind(infd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        fprintf(stderr, "WARNING: cannot bind to socket %s (%s), exiting\n",
                sockpn, strerror(errno));
	exit(-1);
    }

    chmod(sockpn, 0772); //onwer&group: RWX, others: W 
    listen(infd,SOMAXCONN);
    //CONTINUE HERE
    

    return (0);
}
