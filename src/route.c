
#include "route.h"
#include "http.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Route routes[MAX_ROUTE];
int route_count = 0;

int handle_route(http_method_e method, const char *route,
                 void (*fn)(http_request *r, http_response *w)) {

  if (!fn) {
    debug_log("handle fn is NULL\n");
    return route_count;
  }

  if (route_count < MAX_ROUTE) {
    routes[route_count].method = method;
    strcpy(routes[route_count].path, route);
    routes[route_count].handler = fn;
    route_count++;
  }

  return route_count;
}

void sanitize_path(const char *requested_path, char *sanitized_path,
                   size_t buffer_size) {
  const char *web_root = "./www";
  int path_length =
      snprintf(sanitized_path, buffer_size, "%s%s", web_root, requested_path);

  if (strstr(requested_path, "..") != NULL) {
    snprintf(sanitized_path, buffer_size, "%s%s", web_root, "/404.html");
  }
  debug_log("sanitize_path: Sanitized path: %s\n", sanitized_path);
  if (*(sanitized_path + path_length - 1) == '/') {
    debug_log("sanitize_path: Path ends with '/', appending index.html\n");
    strncat(sanitized_path, "index.html",
            buffer_size - strlen(sanitized_path) - 1);
  }
}

void serve_file(const char *path, http_response *response) {
  debug_log("serve_file: Serving file: %s\n", path);
  FILE *file = fopen(path, "rb");
  if (!file) {
    debug_log("serve_file: File not found: %s\n", path);
    response->status_code = 404;
    strncpy(response->reason_phrase, "Not Found", HTTP_MAX_REASON_LEN - 1);
    file = fopen("./www/404.html", "rb");
  }

  // Determine file size
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_content = malloc(file_size);
  if (!file_content) {
    perror("Failed to allocate memory for file content");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fread(file_content, 1, file_size, file);
  fclose(file);

  response->body = file_content;
  response->body_length = file_size;

  // determine content type based on file extension
  if (strstr(path, ".html")) {
    add_http_header(response, "Content-Type", "text/html");
  } else if (strstr(path, ".css")) {
    add_http_header(response, "Content-Type", "text/css");
  } else if (strstr(path, ".js")) {
    add_http_header(response, "Content-Type", "application/javascript");
  } else if (strstr(path, ".png")) {
    add_http_header(response, "Content-Type", "image/png");
  } else if (strstr(path, ".jpg") || strstr(path, ".jpeg")) {
    add_http_header(response, "Content-Type", "image/jpeg");
  } else {
    add_http_header(response, "Content-Type", "application/octet-stream");
  }

  char content_length_str[32];
  snprintf(content_length_str, sizeof(content_length_str), "%zu",
           response->body_length);
  add_http_header(response, "Content-Length", content_length_str);
}
