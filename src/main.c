#include "main.h"
#include "http.h"
#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  // debug_log("Debug mode is enabled.\n");
  //
  // server_status_e status;
  // tcp_server server;
  //
  // status = bind_tcp_port(&server, PORT);
  // if (status != SERVER_OK) {
  //   fprintf(stderr, "Failed to bind TCP port: %d\n", status);
  //   return EXIT_FAILURE;
  // }
  //
  // int client_fd = accept_client(server.socket_fd);
  // if (client_fd == -1) {
  //   fprintf(stderr, "Failed to accept client connection.\n");
  //   close(server.socket_fd);
  //   return EXIT_FAILURE;
  // }
  //
  // debug_log("Client connected\n");
  //
  // http_request request;
  // if (read_http_request(client_fd, &request) == -1) {
  //   debug_log("Failed to read HTTP request\n");
  //   close(client_fd);
  //   close(server.socket_fd);
  //   return EXIT_FAILURE;
  // }
  //
  // printf("Received HTTP request:\n");
  // printf("Method: %s\n", request.method);
  // printf("Path: %s\n", request.path);
  // printf("Protocol: %s\n", request.protocol);

  // close(client_fd);
  // close(server.socket_fd);

  const char *raw_request = "GET /index.html HTTP/1.1\r\n"
                            "Host: localhost:8080\r\n"
                            "User-Agent: curl/7.68.0\r\n"
                            "Accept: */*\r\n"
                            "\r\n";

  http_request request = {0};
  if (parse_http_headers(raw_request, &request) != HTTP_PARSE_OK) {
    fprintf(stderr, "Failed to parse HTTP headers\n");
    return EXIT_FAILURE;
  }

  printf("Parsed HTTP headers:\n");
  for (size_t i = 0; i < request.header_count; i++) {
    printf("%s: %s\n", request.headers[i].key, request.headers[i].value);
  }

  free_http_request(&request);

  return EXIT_SUCCESS;
}
