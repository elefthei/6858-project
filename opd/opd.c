#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include "reqsock/reqlib.h"

int main(int argc, char* argv) {
  system("echo \"$(id -u)\"");

  // Get and bind to uds
  struct sockaddr_un address;
  socklen_t address_length;
  int ss_fd, connection_fd;
  pid_t child;

  ss_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if(ss_fd < 0) {
    printf("socket() failed\n");
    return 1;
  } 

  unlink("./sock");

  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, 7, "./sock");

  if (bind(ss_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
    printf("bind() failed\n");
    return 1;
  }
  system("chmod 777 sock");

  if(listen(ss_fd, 5) != 0) {
    printf("listen() failed\n");
    return 1;
  }


  //listen on socket, handle in a new process when get new connection
  while((connection_fd = accept(ss_fd, (struct sockaddr *) &address, &address_length)) > -1) {
   child = fork();
   if(child == 0) {
     return connection_handler(connection_fd);
    }

   /* still inside server process */
   close(connection_fd);
  }

  close(ss_fd);
  unlink("./sock");
  return 0;
}


int connection_handler(int ud_fd) { // recieve commands from client over ud socket
  printf("connection handler\n");
  int nbytes;
  char buffer[256];
  
  //read thing
  nbytes = read(ud_fd, buffer, 256);

  // get location of fist space, make it null for strcmp
  buffer[nbytes] = 0; //null terminate
  char *sp = strchr(buffer, ' ');
  if (!sp) { //handle if there is no space in the command
    printf("there was no space in command:: %s", buffer);
    return -1;
  }

  *sp = '\0';
  sp++;
 
  int cmd = atoi(buffer);
  if (cmd == BIND) { // run the command given by client
    int port = atoi(sp);
    printf("send_port was success ? :: %d\n", send_port(port, ud_fd));
  } else if (cmd == KILL) {
  }
  else { // command has error, write error message
    printf("invalid command :: %s\n", buffer);
  }

  printf("MESSAGE FROM CLIENT: %s\n", buffer);
  nbytes = snprintf(buffer, 256, "hello from the server");
  write(ud_fd, buffer, nbytes);

  close(ud_fd);
  return 0;
}

int bind_port (int port) {
  printf("bind_port\n");
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
  address.sin_port = htons(port);
  /* bind the socket to the port specified above */
  printf("bind to ss_fd returns %d\n", bind(ss_fd, (struct sockaddr *)&address, sizeof(address)));
  
  return ss_fd;
  /*listen(ss_fd, 3);

  int addrlen = sizeof(struct sockaddr_in);
  int new_socket = accept(ss_fd, (struct sockaddr *)&address, &addrlen);
  if (new_socket<0) {
    perror("Accept connection");
  }
  return new_socket;
  */
}

int send_port(int port, int ud_fd) {
  printf("send_port\n");
  int sock_fd = bind_port(port);
  //now send it over the network
  
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
  int available_ancillary_element_buffer_space;

  /* at least one vector of one byte must be sent */
  message_buffer[0] = 'F';
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;

  /* initialize socket message */
  memset(&socket_message, 0, sizeof(struct msghdr));
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
  memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = available_ancillary_element_buffer_space;

  /* initialize a single ancillary data element for fd passing */
  control_message = CMSG_FIRSTHDR(&socket_message);
  control_message->cmsg_level = SOL_SOCKET;
  control_message->cmsg_type = SCM_RIGHTS;
  control_message->cmsg_len = CMSG_LEN(sizeof(int));
  *((int *) CMSG_DATA(control_message)) = sock_fd;

  return sendmsg(ud_fd, &socket_message, 0);
}
