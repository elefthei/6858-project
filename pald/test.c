#include<stdio.h>
#include "pald_api.h"
#include<unistd.h>

int main(void){
  
  printf("Requesting permission for GID=1000: %d\n",pald_request_gid(1000));
  
  return 0;
}
