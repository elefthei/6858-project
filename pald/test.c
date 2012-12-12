#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(void){
	while(1){
		printf("uid: %d\n",(int)geteuid());
		sleep(1);
	}

	return 0;
}
