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
		perror("Incorrect number of arguments");
		return 1;
	}

	//some check pw
	if(!pald_request_gid( (gid_t)atoi(argv[1]) )) {
		perror("Wrong password");
		return 1;
	}

	int numgroups = getgroups(0, NULL)+1;
	gid_t grouplist[numgroups];
	getgroups(numgroups-1, grouplist);

	grouplist[numgroups-1] = atoi(argv[1]);//some GID attached to this permission
	setgroups(numgroups, grouplist);

	setgid(getgid());

	setuid(getuid());

	execl("/bin/bash", "/bin/bash", (char *) 0);

	return 0;
}