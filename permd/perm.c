#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PORTPWD "jigglypuffs"

int main(int argc, char* argv[]) {
	if(argc!=2) {
		return ; 
	}

	int uid = getuid();
	int numgroups = getgroups(0, NULL)+1;
	gid_t grouplist[numgroups];
	getgroups(numgroups-1, grouplist);
	char pwd[128];
	char* req_perm = argv[1];

	printf("Password: ");
	fflush(stdout);
	system("stty -echo");
	fgets(pwd, sizeof(pwd), stdin);
	system("stty echo");

	if(strchr(pwd, 10)) {
		*strchr(pwd, 10)='\0';
	}

	if(!strcmp(req_perm, "read")) {

	} else if(!strcmp(req_perm, "write")) {

	} else if(!strcmp(req_perm, "port")) { //grant new GID tied to ability to connect to port
  		if(checkpw(pwd, req_perm)) {
  			grouplist[numgroups-1] = 7000;//some group associated with ability to open/close ports
  		}
	} else if(!strcmp(req_perm, "kill")) {
		//grant new GID tied to ability to connect to kill
	}
}

int checkpw(char* input, char* req_perm) {
	char* correct;
	getPwd(req_perm, correct);

	// if(!strcmp(input, correct)) {
	// 	return 1;
	// }
	return 0;
}

void getPwd(char* req_perm, char* correct) {
	FILE* fd = fopen("perm.conf", "r");
	char buf[128];
	char *sp1, *sp2;

	while(fgets(buf, sizeof(buf), fd)) {
		if(strchr(buf, 10)) {
			*strchr(buf, 10)=0;
		}

		if(sp1 = strchr(buf,' ')) {
			*sp1='\0';
			sp1++;

			if(sp2 = strchr(sp1, ' ')) {
				*sp2='\0';
				sp2++;
			}

			if(!strcmp(sp1, req_perm) && !strcmp(buf, "permission")) {
				correct=sp2;
				return;
			}
		}
	}
	correct = NULL;
}