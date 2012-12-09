#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 7000 
// kill things
// bind to port

int main(int argc, char* argv) {
  // create socket
  int ss_fd = socket(AF_INET, SOCK_STREAM, 0); //server socket file descriptor
  printf("%d\n", ss_fd);
  if (ss_fd < -1) {
    perror("Create socket");
  }

  struct sockaddr_in address;
  /* type of socket created in socket() */
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  /* bind the socket to the port specified above */
  printf("bind returns %d\n", bind(ss_fd, (struct sockaddr *)&address, sizeof(address)));

  listen(ss_fd, 3);

  int addrlen = sizeof(struct sockaddr_in);
  int new_socket = accept(ss_fd, (struct sockaddr *)&address, &addrlen);
  if (new_socket<0) {
    perror("Accept connection");
  }

  char buffer[1024];
  int size;
  while ((size = recv(new_socket, buffer, sizeof(buffer) -1, 0)) >= 0) {
    printf("%s\n", buffer);
    send(new_socket, "reply\n\r", 7, 0);



    int i;
    for (i=0; i<1024; i++) {
      buffer[i] = '\0';
    }
  }

}
