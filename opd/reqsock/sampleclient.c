#include <netinet/in.h>
#include "reqlib.h"

int main(void) {
  int port_fd = getPort_fd(79, "../sock", 8);
  printf("port_fd %d\n", port_fd);
  listenToPort(port_fd);
}

int listenToPort(int port_fd) {
  struct sockaddr_in address;
  printf("listen to port :: %d\n", listen(port_fd, 3));

  int addrlen = sizeof(struct sockaddr_in);
  int new_socket = accept(port_fd, (struct sockaddr *)&address, &addrlen);
  printf("new socekt :: %d\n", new_socket);
  if (new_socket < 0) {
    perror("Accept connection");
  }

  char buffer[1024];
  int size;
  printf("about to listen\n"); 
  while ((size = recv(new_socket, buffer, sizeof(buffer) -1, 0)) >= 0) {
    printf("%s\n", buffer);
    send(new_socket, "reply\n\r", 7, 0);

    memset(buffer, 0, 1024);
  }
}
