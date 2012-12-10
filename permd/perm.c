#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PORTPWD "jigglypuff"

int main(int argc, char* argv[]) {
	int uid = getuid();
	int numgroups = getgroups(0, NULL)+1;
	gid_t grouplist[numgroups];
	getgroups(numgroups-1, grouplist);
	char pwd[128];

	if(argc!=2) { //change later? need it for now
		return ; 
	}

	printf("%d\n", uid);
	// printf("%d::%d::%d::%d::%d::%d::%d::%d\n", grouplist[0], grouplist[1], grouplist[2], grouplist[3], grouplist[4], grouplist[5], grouplist[6], grouplist[7]);
	if(!strcmp(argv[1], "port")) {
		//grant new GID tied to ability to connect to port
		printf("Password:\n");
		char *envp[] = { NULL };
  		char *args[] = { "/bin/stty", "-echo",NULL};
  		fflush(stdout);
  		execve("/bin/stty", args, envp);
  		char *p;
  		fgets(pwd, sizeof(pwd), stdin);
  		// if ((p = strchr(pwd, '\n')) != NULL) {   /* check if last element is a newline */
	     	// *p = '\0';                          /* make last element a null character */
    	// }
    	
  		// args[1] = "echo";
  		// execve("/bin/stty", args, envp);
  		printf("Printing now::%s\n", pwd);
	} else if(!strcmp(argv[1], "kill")) {
		//grant new GID tied to ability to connect to kill
	}
}