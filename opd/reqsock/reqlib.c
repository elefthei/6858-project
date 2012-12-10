#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "reqlib.h"


int getPort_fd(int port, const char* path, int len) {
  int ud_fd = ctd(path, len);
  request_port(ud_fd, port);
  int port_fd = receive_port(ud_fd);

  return port_fd;
}

int ctd(const char* path, int len) { //connect to domain socket

  //connects to the socket 
  struct sockaddr_un address;
  int  ud_fd;

  ud_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if(ud_fd < 0) {
    printf("socket() failed\n");
    return -1;
  }

  /* start with a clean address structure */
  memset(&address, 0, sizeof(struct sockaddr_un));

  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, len, path);

  if(connect(ud_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
    printf("connect() failed\n");
    return -1;
  }

  return ud_fd;
}

int request_port(int ud_fd, int port) {
    char msg[256];
    int nbytes = snprintf(msg, 256, "%d %d", BIND, port);
    return write(ud_fd, msg, nbytes);
}

int receive_port(int ud_fd) {
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
