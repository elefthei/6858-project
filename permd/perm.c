#include <sys/types.h>
#include <stdio.h>
#include <grp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pwd.h>
#include "../pald/pald_api.h"

int main(int argc, char* argv[]) {
	if(argc!=2) {
		return ; 
	}
	// char pwd[128];
	// char buf[256];
	// char* sp;

	// fflush(stdout);
	// printf("Password: ");
	// system("stty -echo");
	// fgets(pwd, sizeof(pwd), stdin);
	// system("stty echo");
	// printf("\n");

	// if(strchr(pwd, 10)) {
	// 	*strchr(pwd, 10)='\0';
	// }

	//some check pw
	if(!pald_request_gid(argv[1])) {
		return ;
	}

	int numgroups = getgroups(0, NULL)+1;
	gid_t grouplist[numgroups];
	getgroups(numgroups-1, grouplist);

	
	//setuid(system("echo $SUDO_UID"));

	grouplist[numgroups-1] = argv[1];//some GID attached to this permission
	setgroups(numgroups, grouplist);

	// printf("::id pre-anything::\n");
	// system("id");

	setgid(atoi(getenv("SUDO_UID")));
	// printf("::id post-setgid::\n");
	//system("id");

	setuid(atoi(getenv("SUDO_UID")));
	// printf("::id post-setuid::\n");
	// system("id");


	execl("/bin/sh", "/bin/sh", (char *) 0);
}