#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

int main(int argc, char* argv[]) {
  //connects to the socket 
  struct sockaddr_un address;
  int  ud_fd, nbytes;
  char buffer[256];
  memset(&buffer, 0, 256);

  ud_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if(ud_fd < 0)
  {
    printf("socket() failed\n");
    return 1;
  }

  /* start with a clean address structure */
  memset(&address, 0, sizeof(struct sockaddr_un));

  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, 7, "./sock");

  if(connect(ud_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
    printf("connect() failed\n");
    return 1;
  }

  // send a command to the daemon
  if (argc > 1) {
    printf("argc is %d argv[1] is %s\n", argc, argv[1]);
    printf("size of argv1 %lu\n", strlen(argv[1]));
    printf("argv[1] == bind ?:: %d\n", strcmp(argv[1], "bind"));

    nbytes = strlen(argv[1]) + strlen(argv[2]) + 1;
    strncpy(buffer, argv[1], strlen(argv[1]));
    
    strcat(buffer, " ");
    strcat(buffer, argv[2]);
    buffer[nbytes] = '\0';
    //printf("buffer :: %s \t nbytes :: %d\n", buffer, nbytes);
    //printf("buffer[5] :: %di\n", buffer[5]); 
    
  } else {
    printf("argc is %d\n", argc); 
    nbytes = snprintf(buffer, 256, "hello from a client");
  }
  
  write(ud_fd, buffer, nbytes);
  int port_fd = get_fd(ud_fd);
  listenToPort(port_fd);

  /*nbytes = read(ud_fd, buffer, 256);
  buffer[nbytes] = 0;

  printf("MESSAGE FROM SERVER: %s\n", buffer);
  */
  close(ud_fd);

  return 0;
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


int get_fd(int ud_fd) {
  int sent_fd, available_ancillary_element_buffer_space;
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

  /* start clean */
  memset(&socket_message, 0, sizeof(struct msghdr));
  memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

  /* setup a place to fill in message contents */
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

  if(recvmsg(ud_fd, &socket_message, MSG_CMSG_CLOEXEC) < 0)
    return -1;

  if(message_buffer[0] != 'F')
  {
    /* this did not originate from the above function */
    return -1;
  }

  if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
  {
    /* we did not provide enough space for the ancillary element array */
    return -1;
  }

  /* iterate ancillary elements */
  for(control_message = CMSG_FIRSTHDR(&socket_message);
      control_message != NULL;
      control_message = CMSG_NXTHDR(&socket_message, control_message)) {

    if( (control_message->cmsg_level == SOL_SOCKET) &&
        (control_message->cmsg_type == SCM_RIGHTS) ) {
      sent_fd = *((int *) CMSG_DATA(control_message));
      return sent_fd;
    }
  }
  return -1;
}
