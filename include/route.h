#ifndef ROUTE_H
#define ROUTE_H

#include "http.h"
#include <stdbool.h>

typedef struct {
  http_request *request;
  http_response *response;
  int client_fd;
} route_data_t;

void sanitize_path(const char *requested_path, char *sanitized_path,
                   size_t buffer_size);
void serve_file(const char *path, http_response *response);
void handle_route(const char *route,
                  bool (*fn)(http_request *r, http_response *w),
                  route_data_t route_data);

#endif // !ROUTE_H
