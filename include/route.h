#ifndef ROUTE_H
#define ROUTE_H

#include "http.h"
#include <stdbool.h>

#define MAX_ROUTE 10

typedef struct {
  http_method_e method;
  char path[HTTP_PATH_MAX_LEN];
  void (*handler)(http_request *, http_response *);
} Route;

typedef struct {
  http_request *request;
  http_response *response;
  int client_fd;
} route_data_t;

void sanitize_path(const char *requested_path, char *sanitized_path,
                   size_t buffer_size);
void serve_file(const char *path, http_response *response);
int handle_route(http_method_e method, const char *route,
                 void (*fn)(http_request *r, http_response *w));

#endif // !ROUTE_H
