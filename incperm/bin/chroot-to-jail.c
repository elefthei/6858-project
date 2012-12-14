#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char* argv[]) {
 
  if (argc < 2) {
    printf("need the original uid as an argument\n");
    return -1;
  }
  int origuid = atoi(argv[1]);
  char buffer[256];

  snprintf(buffer, 256, "/srv/incperm/incperm-%d", origuid);

  chdir(buffer);
  if(chroot(buffer) != 0) {
    perror("chroot error\n");
  }
  setuid(origuid);
  printf("getuid %d\n", getuid());
  execl("/bin/sh", "/bin/sh", (char *) 0);
  
  return 0;
}
