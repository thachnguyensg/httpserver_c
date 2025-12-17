#include "main.h"
#include "http.h"
#include "route.h"
#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handle_hello(http_request *req, http_response *res) {
  debug_log("handle_root called\n");
  res->status_code = 200;

  if (!res->body) {
    res->body = malloc(64);
  }
  char *payload = "Hello hello";
  strcpy(res->body, payload);
  res->body_length = strlen(payload);
  char clen[20];
  sprintf(clen, "%zu", res->body_length);
  add_http_header(res, "Content-Length", clen);
}

int main() {
  debug_log("Debug mode is enabled.\n");

  server_status_e status;
  tcp_server server;

  status = bind_tcp_port(&server, PORT);
  if (status != SERVER_OK) {
    fprintf(stderr, "Failed to bind TCP port: %d\n", status);
    return EXIT_FAILURE;
  }

  while (1) {
    int client_fd = accept_client(server.socket_fd);
    if (client_fd == -1) {
      fprintf(stderr, "Failed to accept client connection.\n");
      close(server.socket_fd);
      return EXIT_FAILURE;
    }

    debug_log("Client connected\n");

    http_request request = {0};
    if (read_http_request(client_fd, &request) == HTTP_PARSE_INVALID) {
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

    http_response response;
    init_http_response(&response);

    handle_route(HTTP_METHOD_GET, "/hello", &handle_hello);

    char sanitized_path[1024];
    sanitize_path(request.path, sanitized_path, sizeof(sanitized_path));
    if (!handle_request(&request, &response)) {
      serve_file(sanitized_path, &response);
    }

    send_http_response(client_fd, &response);

    free_http_headers(&request);
    free_http_response(&response);
    close(client_fd);
    debug_log("Response sent and client disconnected\n");
  }

  close(server.socket_fd);

  return EXIT_SUCCESS;
}
