#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if(argc!=2) {
		return ; 
	}
	char pwd[128];
	char poop[128];

	fflush(stdout);
	printf("Password: ");
	system("stty -echo");
	fgets(pwd, sizeof(pwd), stdin);
	system("stty echo");
	printf("\n");

	if(strchr(pwd, 10)) {
		*strchr(pwd, 10)='\0';
	}

	//some check pw
	// if(!checkpw(argv[1], pwd)) {
	// 	return ;
	// }

	int numgroups = getgroups(0, NULL);
	gid_t grouplist[numgroups];
	getgroups(numgroups, grouplist);

	if(!strcmp(argv[1], "port")) {
		//system("groups scooby");
		//setuid(system("echo $SUDO_UID"));

		grouplist[0] = 1001;
		setgroups(numgroups, grouplist);

		printf("::id pre-anything::\n");
		system("id");

		setgid(atoi(getenv("SUDO_UID")));
		printf("::id post-setgid::\n");
		system("id");

		setuid(atoi(getenv("SUDO_UID")));
		printf("::id post-setuid::\n");
		system("id");
		execl("/bin/sh", "/bin/sh", (char *) 0);
	}
}