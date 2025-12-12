#include "tcp.h"
#include "main.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

server_status_e bind_tcp_port(tcp_server *server, int port) {
  memset(server, 0, sizeof(*server));

  if (port > 65535) {
    debug_log("Invalid port");
    return SERVER_BIND_ERROR;
  }

  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd == -1) {
    perror("socket");
    return SERVER_SOCKET_ERROR;
  }

  server->address.sin_family = AF_INET;
  server->address.sin_addr.s_addr = INADDR_ANY;
  server->address.sin_port = htons(port);

  if (bind(server->socket_fd, (struct sockaddr *)&server->address,
           sizeof(server->address)) == -1) {
    perror("bind");
    close(server->socket_fd);
    return SERVER_BIND_ERROR;
  }

  if (listen(server->socket_fd, 5) == -1) {
    perror("listen");
    close(server->socket_fd);
    return SERVER_LISTEN_ERROR;
  }

  debug_log("Server listening on port %d\n", port);

  return SERVER_OK;
}

int accept_client(int server_fd) {
  struct sockaddr_in client_address = {0};
  socklen_t client_len = sizeof(client_address);

  int client_fd =
      accept(server_fd, (struct sockaddr *)&client_address, &client_len);
  if (client_fd == -1) {
    perror("accept");
    return -1;
  }

  return client_fd;
}
