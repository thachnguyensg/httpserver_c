#include "main.h"
#include "http.h"
#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  debug_log("Debug mode is enabled.\n");

  server_status_e status;
  tcp_server server;

  status = bind_tcp_port(&server, PORT);
  if (status != SERVER_OK) {
    fprintf(stderr, "Failed to bind TCP port: %d\n", status);
    return EXIT_FAILURE;
  }

  int client_fd = accept_client(server.socket_fd);
  if (client_fd == -1) {
    fprintf(stderr, "Failed to accept client connection.\n");
    close(server.socket_fd);
    return EXIT_FAILURE;
  }

  debug_log("Client connected\n");

  http_request request = {0};
  if (read_http_request(client_fd, &request) == -1) {
    debug_log("Failed to read HTTP request\n");
    close(client_fd);
    close(server.socket_fd);
    return EXIT_FAILURE;
  }

  if (parse_http_request(request.buffer, &request) != HTTP_PARSE_OK) {
    debug_log("Failed to parse HTTP request\n");
    free_http_headers(&request);
    close(client_fd);
    close(server.socket_fd);
    return EXIT_FAILURE;
  }

  printf("Received HTTP request:\n");
  printf("Method: %s\n", request.method);
  printf("Path: %s\n", request.path);
  printf("Protocol: %s\n", request.protocol);

  printf("Parsed HTTP headers:\n");
  for (size_t i = 0; i < request.header_count; i++) {
    printf("%s: %s\n", request.headers[i].key, request.headers[i].value);
  }

  free_http_headers(&request);
  close(client_fd);
  close(server.socket_fd);

  return EXIT_SUCCESS;
}
