#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PORTPWD "jigglypuffs"

int main(int argc, char* argv[]) {
	int uid = getuid();
	int numgroups = getgroups(0, NULL)+1;
	gid_t grouplist[numgroups];
	getgroups(numgroups-1, grouplist);
	char pwd[128];

	if(argc!=2) { //change later? need it for now
		return ; 
	}

	if(!strcmp(argv[1], "port")) { //grant new GID tied to ability to connect to port
		printf("Password: ");
  		fflush(stdout);
  		system("stty -echo");
  		fgets(pwd, sizeof(pwd), stdin);
  		system("stty echo");

    	if(strchr(pwd, 10)) {
    		*strchr(pwd, 10)=0;
    	}
  		if(checkpw(pwd, PORTPWD)) {

  		}
	} else if(!strcmp(argv[1], "kill")) {
		//grant new GID tied to ability to connect to kill
	}
}

int checkpw(char* input, char* correct) {
	if(!strcmp(input, correct)) {
		return 1;
	}
	return 0;
}