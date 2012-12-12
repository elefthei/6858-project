#include<unistd.h>
#include<stdlib.h>

#ifndef PALD_API_CALLS
#define PALD_API_CALLS
#endif

#define PALD_READ 0
#define PALD_WRITE 1
#define PALD_PORT 2
#define PALD_KILL 3

int pald_get_perm(const unsigned int req_perm, gid_t GID){

  char command[];
  sprintf(command,"python sclient.py %d", GID);
  return system(command);
  
}
