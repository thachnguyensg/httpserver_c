#include "http.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int read_http_request(int socket_fd, http_request *request) {
  char buf[8192] = {0};
  ssize_t bytes = recv(socket_fd, buf, sizeof(buf) - 1, 0);
  if (bytes <= 0) {
    perror("recv");
    return -1;
  }

  buf[bytes] = '\0';

  if (sscanf(buf, "%7s %2047s %15s", request->method, request->path,
             request->protocol) != 3) {
    fprintf(stderr, "Failed to parse HTTP request\n");
    return -1;
  }

  return 0;
}
